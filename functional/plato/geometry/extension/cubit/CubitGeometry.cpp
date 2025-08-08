#include "plato/geometry/extension/cubit/CubitGeometry.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <concepts>
#include <filesystem>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

#include "plato/analysis/Utilities.hpp"
#include "plato/geometry/extension/cubit/CubitGeometryCommonUtilities.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/third_party_integration/common/Vector3Serialization.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/HashUtilities.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/ReduceUtilities.hpp"

namespace plato::geometry::extension::cubit
{

namespace
{
const auto kComputeMesh = third_party_integration::cubit::ApreproVariable{"compute_mesh", 1};
// LCOV_EXCL_START
[[nodiscard]] auto make_cubit_output(const input_parser::cubit_parameterized_shape& aInput)
    -> std::function<void(const linear_algebra::DynamicVector<double>&, const library::OutputInfo&)>
{
    return [aInput](const linear_algebra::DynamicVector<double>& aSolution, const library::OutputInfo& aOutputInfo)
    {
        utilities::execute_on_root(
            boost::mpi::communicator{},
            [&aInput, &aSolution, &aOutputInfo]()
            {
                if (aInput.output_mesh_sensitivities_name.has_value())
                {
                    auto tCubitGeometry = CubitGeometry{aInput};
                    const auto tAnalysisDomainMesh = tCubitGeometry.generateMesh(aSolution);

                    const auto tMeshOutput = mesh::make_mesh_output(
                        mesh::output_mode(aOutputInfo.mOverwrite), mesh::InputFilePath{tAnalysisDomainMesh.mFileName},
                        mesh::OutputFilePath{aInput.output_mesh_sensitivities_name.value().mToken}, {},
                        aOutputInfo.mIteration);

                    tCubitGeometry.outputMeshSensitivities(*tMeshOutput);
                }
                cubit::aprepro_variable_output(aInput.output_file_name.value().mToken, aSolution,
                                               cubit::initialize_variables(aInput));
            });
    };
}

[[nodiscard]] auto make_cubit_geometry(const input_parser::cubit_parameterized_shape& aInput)
    -> library::GeometryFunction
{
    const auto tCubitGeometry = std::make_shared<CubitGeometry>(aInput);
    return library::GeometryFunction{[tCubitGeometry](const linear_algebra::DynamicVector<double>& x)
                                     { return tCubitGeometry->generateMesh(x); },
                                     [tCubitGeometry](const linear_algebra::DynamicVector<double>& x)
                                     { return tCubitGeometry->jacobianMultiplier(x); },
                                     [tCubitGeometry](const linear_algebra::DynamicVector<double>& x)
                                     { return tCubitGeometry->adjointJacobianMultiplier(x); }};
}

/// Static registration for the input parser
[[maybe_unused]] static auto kCubitGeometryParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::cubit_parameterized_shape>{};

[[maybe_unused]] static auto kCubitGeometryRegistration = geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::cubit_parameterized_shape>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::cubit_parameterized_shape>(aGeometryInput);
        return library::FactoryTypes{make_cubit_geometry(tInput), cubit::initial_guess(tInput), cubit::bounds(tInput),
                                     make_cubit_output(tInput)};
    }};

[[maybe_unused]] static auto kCubitGeometryValidationRegistration =
    input_validation::ValidationRegistration<input_parser::cubit_parameterized_shape>{
        [](const input_parser::cubit_parameterized_shape& aInput)
        { return detail::validate_mesh_journal_file(aInput); },
        [](const input_parser::cubit_parameterized_shape& aInput) { return validate_mesh_file_name(aInput); },
        [](const input_parser::cubit_parameterized_shape& aInput) { return validate_output_file_name(aInput); },
        [](const input_parser::cubit_parameterized_shape& aInput)
        { return validate_lower_bounds_less_than_upper(aInput); },
        [](const input_parser::cubit_parameterized_shape& aInput)
        { return validate_aprepro_variables_exists(aInput); }};
// LCOV_EXCL_STOP
}  // namespace

CubitGeometry::CubitGeometry(const plato::input_parser::cubit_parameterized_shape& aInput)
    : mBaseJournalFile(aInput.mesh_journal_file.value().mToken),
      mMeshFile(aInput.mesh_file_name.value().mToken),
      mVariables(initialize_variables(aInput)),
      mMeshCache(MeshCache{[this](const linear_algebra::DynamicVector<double>& aDesignParameter)
                           { return this->generateMesh(aDesignParameter); },
                           [](const linear_algebra::DynamicVector<double>& aDesignParameter)
                           { return utilities::hash_container(aDesignParameter.stdVector()); }})
{
}

auto CubitGeometry::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter)
    -> analysis::AnalysisDomainMesh
{
    utilities::execute_on_root(boost::mpi::communicator{},
                               [this, &aDesignParameter]()
                               {
                                   mCubit.addApreproVariable(kComputeMesh);
                                   update_variables(mVariables, aDesignParameter);
                                   detail::write_exodus_mesh(mCubit, mVariables, detail::JournalFile{mBaseJournalFile},
                                                             detail::ExodusFile{mMeshFile});
                               });
    return analysis::AnalysisDomainMesh{mMeshFile, {}};
}

namespace
{

const auto kXAccessor = [](const third_party_integration::common::Vector3& aVector3) { return aVector3.x; };
const auto kYAccessor = [](const third_party_integration::common::Vector3& aVector3) { return aVector3.y; };
const auto kZAccessor = [](const third_party_integration::common::Vector3& aVector3) { return aVector3.z; };

template <typename Function>
concept AccessorFunction = requires(Function aFunction) {
    { aFunction(third_party_integration::common::Vector3{}) } -> std::convertible_to<double>;
};

template <AccessorFunction AccessorFunction>
[[nodiscard]] auto sensitivity_to_analysis_domain_mesh(analysis::AnalysisDomainMesh aAnalysisDomainMesh,
                                                       const CubitSensitivityMap& aSensitivityMap,
                                                       const AccessorFunction aAccessorFunction)
    -> analysis::AnalysisDomainMesh
{
    auto tAnalysisDomainMesh = aAnalysisDomainMesh;
    for (auto& tBlock : tAnalysisDomainMesh.mBlockScalarField)
    {
        for (auto& tEntry : tBlock.second)
        {
            const auto tID = static_cast<third_party_integration::cubit::CubitGlobalId>(tEntry.mGlobalMeshEntityID);
            tEntry.mValue = 0.0;
            if (const auto tSensitivityAtID = aSensitivityMap.find(tID); tSensitivityAtID != aSensitivityMap.end())
            {
                tEntry.mValue = aAccessorFunction(tSensitivityAtID->second);
            }
        }
    }
    return tAnalysisDomainMesh;
}

}  // namespace

void CubitGeometry::outputMeshSensitivities(mesh::MeshOutput& aMeshOutput)
{
    const auto tField = std::vector<double>(mesh::EntityCounts{aMeshOutput}.numberOfNodes(), 0.0);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{
        mesh::Mesh{mMeshFile}}.nodalFieldToAnalysisDomainMesh(mesh::NodalFieldVectorReference{tField});

    const auto tAllSurfaceSensitivities = detail::sensitivities(mCubit, mVariables, mBaseJournalFile);
    const auto tPostFixNames = std::vector<std::string>{"_x", "_y", "_z"};
    const auto tComponentAccessorFunctions =
        std::vector<std::function<double(const third_party_integration::common::Vector3&)>>{kXAccessor, kYAccessor,
                                                                                            kZAccessor};

    for (const auto& [tDesignIndex, tSensitivityMap] : utilities::enumerate(tAllSurfaceSensitivities))
    {
        for (const auto [tPostFixName, tComponentAccessor] : utilities::Zip(tPostFixNames, tComponentAccessorFunctions))
        {
            auto tAnalysisDomainMeshSensitivity =
                sensitivity_to_analysis_domain_mesh(tAnalysisDomainMesh, tSensitivityMap, tComponentAccessor);
            constexpr auto tFixedValue = double{0};
            aMeshOutput.addFieldOnAnalysisDomainMesh(tAnalysisDomainMeshSensitivity,
                                                     mVariables.at(tDesignIndex).mName + tPostFixName, tFixedValue);
        }
    }
}

namespace
{

constexpr auto kXComponent = utilities::ComponentIndex{0};
constexpr auto kYComponent = utilities::ComponentIndex{1};
constexpr auto kZComponent = utilities::ComponentIndex{2};

[[nodiscard]] auto row_vector_to_vector3(const std::vector<double>& aRowVector,
                                         const utilities::VectorIndex aVectorIndex,
                                         const std::size_t aDimensions) -> third_party_integration::common::Vector3
{
    const auto tRowVectorView = utilities::make_multi_vector_view(aRowVector, aDimensions);
    return third_party_integration::common::Vector3{
        tRowVectorView(aVectorIndex, kXComponent), tRowVectorView(aVectorIndex, kYComponent),
        aDimensions == 3U ? tRowVectorView(aVectorIndex, kZComponent) : 0.0};
}

using ResultSize = utilities::NamedType<long unsigned int, struct ResultSizeTag>;
using ResultViewDimensionality = utilities::NamedType<std::size_t, struct ResultViewDimensionalityTag>;
using SpatialDimensions = utilities::NamedType<std::size_t, struct SpatialDimensionsTag>;
using DesignIndex = utilities::NamedType<long unsigned int, struct DesignIndexTag>;
using MeshIndex = utilities::NamedType<long unsigned int, struct MeshIndexTag>;

const auto kJacobianImpl = [](utilities::MultiVectorView<std::vector<double>>& aResultVectorView,
                              const std::vector<double>& aRowVector,
                              const DesignIndex aDesignIndex,
                              const MeshIndex aMeshIndex,
                              const third_party_integration::common::Vector3& aSensitivity,
                              const SpatialDimensions aSpatialDimensions)
{
    constexpr auto kScalarViewComponent = utilities::ComponentIndex{0};
    aResultVectorView(utilities::VectorIndex{aDesignIndex.mValue}, kScalarViewComponent) +=
        third_party_integration::common::dot(
            row_vector_to_vector3(aRowVector, utilities::VectorIndex{aMeshIndex.mValue}, aSpatialDimensions.mValue),
            aSensitivity);
};

const auto kAdjointJacobianImpl = [](utilities::MultiVectorView<std::vector<double>>& aResultVectorView,
                                     const std::vector<double>& aRowVector,
                                     const DesignIndex aDesignIndex,
                                     const MeshIndex aMeshIndex,
                                     const third_party_integration::common::Vector3& aSensitivity,
                                     const SpatialDimensions aSpatialDimensions)
{
    aResultVectorView(utilities::VectorIndex{aMeshIndex.mValue}, kXComponent) +=
        aRowVector[aDesignIndex.mValue] * aSensitivity.x;
    aResultVectorView(utilities::VectorIndex{aMeshIndex.mValue}, kYComponent) +=
        aRowVector[aDesignIndex.mValue] * aSensitivity.y;
    if (aSpatialDimensions.mValue == 3U)
    {
        aResultVectorView(utilities::VectorIndex{aMeshIndex.mValue}, kZComponent) +=
            aRowVector[aDesignIndex.mValue] * aSensitivity.z;
    }
};

template <typename Function>
concept JacobianOrAdjointJacobianFunction = requires(Function aFunction,
                                                     utilities::MultiVectorView<std::vector<double>> aResultVectorView,
                                                     std::vector<double> aRowVector,
                                                     DesignIndex aDesignIndex,
                                                     MeshIndex aMeshIndex,
                                                     third_party_integration::common::Vector3 aSensitivity,
                                                     SpatialDimensions aSpatialDimensions) {
    { aFunction(aResultVectorView, aRowVector, aDesignIndex, aMeshIndex, aSensitivity, aSpatialDimensions) };
};

template <JacobianOrAdjointJacobianFunction JacobianOrAdjointJacobianFunction>
[[nodiscard]] auto transformSensitivityMap(const std::vector<double>& aRowVector,
                                           const ApreproVariableSensitivities& aApreproVariableSensitivities,
                                           const std::vector<size_t>& aNodeIds,
                                           const ResultSize aResultSize,
                                           const ResultViewDimensionality aResultViewDimensionality,
                                           const SpatialDimensions aSpatialDimension,
                                           const JacobianOrAdjointJacobianFunction& aApplyFunction)
    -> std::vector<double>
{
    auto tRowVectorMatrixProduct = std::vector<double>(aResultSize.mValue, 0.0);
    auto tRowVectorMatrixProductView =
        utilities::make_multi_vector_view(tRowVectorMatrixProduct, aResultViewDimensionality.mValue);

    for (const auto [tIndex, tMeshId] : utilities::enumerate(aNodeIds))
    {
        for (const auto [tDesignIndex, tSensitivityMap] : utilities::enumerate(aApreproVariableSensitivities))
        {
            if (const auto tSensitivityMapAtMeshIdIterator =
                    tSensitivityMap.find(static_cast<third_party_integration::cubit::CubitGlobalId>(tMeshId));
                tSensitivityMapAtMeshIdIterator != tSensitivityMap.end())
            {
                const auto& tSensitivity = tSensitivityMapAtMeshIdIterator->second;

                aApplyFunction(tRowVectorMatrixProductView, aRowVector, DesignIndex{tDesignIndex}, MeshIndex{tIndex},
                               tSensitivity, aSpatialDimension);
            }
        }
    }

    return tRowVectorMatrixProduct;
}

}  // namespace

auto CubitGeometry::jacobianMultiplier(const linear_algebra::DynamicVector<double>& aDesignParameter)
    -> linear_algebra::JacobianMultiplier
{
    const auto tCommunicator = boost::mpi::communicator{};
    utilities::execute_on_root(tCommunicator,
                               [this, &aDesignParameter]() { update_variables(mVariables, aDesignParameter); });
    const auto tNodeIds = mesh::EntityRetrieval{mesh::Mesh{mMeshFile}}.designDomainNodeIDs();
    const auto tSpatialDimension = mesh::EntityCounts{mesh::Mesh{mMeshFile}}.spatialDimensions();

    const auto tAllSurfaceSensitivities = utilities::compute_on_root<ApreproVariableSensitivities>(
        tCommunicator, [this]() { return detail::sensitivities(mCubit, mVariables, mBaseJournalFile); });

    return linear_algebra::JacobianMultiplier{
        [tAllSurfaceSensitivities, tSpatialDimension,
         tNodeIds](const linear_algebra::DynamicVector<double>& aRowVector) -> linear_algebra::DynamicVector<double>
        {
            const auto tJacobianResultViewSize = 1U;
            const auto tRowVectorMatrixProduct = transformSensitivityMap(
                aRowVector.stdVector(), tAllSurfaceSensitivities, tNodeIds, ResultSize{tAllSurfaceSensitivities.size()},
                ResultViewDimensionality{tJacobianResultViewSize}, SpatialDimensions{tSpatialDimension}, kJacobianImpl);
            return linear_algebra::DynamicVector<double>{tRowVectorMatrixProduct};
        }};
}

auto CubitGeometry::adjointJacobianMultiplier(const linear_algebra::DynamicVector<double>& aDesignParameter)
    -> linear_algebra::AdjointJacobianMultiplier
{
    const auto tCommunicator = boost::mpi::communicator{};
    utilities::execute_on_root(tCommunicator,
                               [this, &aDesignParameter]() { update_variables(mVariables, aDesignParameter); });

    const auto tSpatialDimension = mesh::EntityCounts{mesh::Mesh{mMeshFile}}.spatialDimensions();
    const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{mMeshFile}}.numberOfDesignDomainNodes();
    const auto tResultSize = tNumberOfNodes * tSpatialDimension;

    const auto tNodeIds = mesh::EntityRetrieval{mesh::Mesh{mMeshFile}}.designDomainNodeIDs();

    const auto tAllSurfaceSensitivities = utilities::compute_on_root<ApreproVariableSensitivities>(
        tCommunicator, [this]() { return detail::sensitivities(mCubit, mVariables, mBaseJournalFile); });

    return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{
        [tAllSurfaceSensitivities, tSpatialDimension, tResultSize,
         tNodeIds](const linear_algebra::DynamicVector<double>& aRowVector) -> linear_algebra::DynamicVector<double>
        {
            const auto tJacobianResultViewSize = tSpatialDimension;
            const auto tRowVectorMatrixProduct =
                transformSensitivityMap(aRowVector.stdVector(), tAllSurfaceSensitivities, tNodeIds,
                                        ResultSize{tResultSize}, ResultViewDimensionality{tJacobianResultViewSize},
                                        SpatialDimensions{tSpatialDimension}, kAdjointJacobianImpl);
            return linear_algebra::DynamicVector<double>{tRowVectorMatrixProduct};
        }}};
}

namespace detail
{

void write_exodus_mesh(CubitWrapper& aCubit,
                       const std::vector<PerturbableApreproVariable>& aVariables,
                       const JournalFile& aJournalFile,
                       const ExodusFile& aExodusFile)
{
    restore_aprepro_variables(aCubit, aVariables);
    aCubit.reset();
    aCubit.playJournalFile(aJournalFile.mValue);
    aCubit.exportExodusFile(aExodusFile.mValue);
}

auto sensitivities(CubitWrapper& aCubit,
                   const std::vector<PerturbableApreproVariable>& aVariables,
                   const std::filesystem::path& aJournalFile) -> ApreproVariableSensitivities
{
    ApreproVariableSensitivities tSensitivities;
    tSensitivities.reserve(aVariables.size());
    std::ranges::transform(
        aVariables, std::back_inserter(tSensitivities),
        [&aCubit, aJournalFile, aVariables](const auto aVariable)
        {
            restore_aprepro_variables(aCubit, aVariables);
            const auto tVariable =
                third_party_integration::cubit::ApreproVariable{aVariable.mName, aVariable.mValue.mValue};
            return aCubit.sensitivities(aJournalFile, tVariable, aVariable.mPerturbationScale.mValue);
        });

    return tSensitivities;
}

std::optional<std::string> validate_mesh_journal_file(const input_parser::cubit_parameterized_shape& aInput)
{
    return input_validation::error_message_for_missing_file_parameter(
        input_parser::block_name<input_parser::cubit_parameterized_shape>(), aInput.mesh_journal_file,
        "mesh_journal_file");
}

}  // namespace detail
}  // namespace plato::geometry::extension::cubit
