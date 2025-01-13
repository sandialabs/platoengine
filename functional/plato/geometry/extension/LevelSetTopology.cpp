#include "plato/geometry/extension/LevelSetTopology.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/analysis/Utilities.hpp"
#include "plato/core/Compose.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/geometry/library/GeometryFilterUtilities.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/DesignVariableAdapter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/third_party_integration/krino/Interface.hpp"
#include "plato/third_party_integration/krino/SphereFactory.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/ParameterBounds.hpp"

namespace plato::geometry::extension
{

namespace
{
namespace tpik = third_party_integration::krino;

constexpr auto kDimensions = std::size_t{3};
constexpr double kLevelSetFixedValue = 1.0;
constexpr auto kTopologyFieldName = std::string_view{"Topology"};
constexpr auto kKrinoLogFileName = std::string_view{"Krino_Output.txt"};
constexpr auto kKrinoCutMeshBaseName = std::string_view{"krino_cut_mesh.exo"};

constexpr auto kXComponent = utilities::ComponentIndex{0};
constexpr auto kYComponent = utilities::ComponentIndex{1};
constexpr auto kZComponent = utilities::ComponentIndex{2};

constexpr auto kMeshNameAccessor =
    [](const input_parser::level_set_topology& aInput) -> const boost::optional<input_parser::FileName>&
{ return aInput.background_mesh_name; };

auto background_mesh_name(const input_parser::level_set_topology& aInput) -> const std::string&
{
    return aInput.background_mesh_name.value().mToken;
}

auto mesh_from_input(const input_parser::level_set_topology& aInput) -> mesh::Mesh
{
    assert(kMeshNameAccessor(aInput).has_value());
    return mesh::Mesh{background_mesh_name(aInput), fixed_blocks(aInput)};
}

auto make_topology_output(const input_parser::level_set_topology& aInput)
    -> std::function<void(const linear_algebra::DynamicVector<double>&)>
{
    return [aInput](const linear_algebra::DynamicVector<double>& aSolution)
    { return LevelSetTopology::output(aInput, library::make_filter_from_geometry_input(aInput), aSolution); };
}

void initialize_krino()
{
    static bool tIsInitialized = false;
    if (!tIsInitialized)
    {
        tIsInitialized = true;
        tpik::initialize_environment_for_krino(kKrinoLogFileName, MPI_COMM_WORLD);
    }
}

auto make_level_set_geometry(const input_parser::level_set_topology& aLevelSetTopology) -> library::GeometryFunction
{
    return make_level_set_geometry(std::make_shared<LevelSetTopology>(aLevelSetTopology),
                                   library::make_filter_from_geometry_input(aLevelSetTopology));
}

/// Static registration for library
[[maybe_unused]] static auto kLevelSetTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::level_set_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        initialize_krino();
        const auto& tInput = core::validated_variant_raw_input<input_parser::level_set_topology>(aGeometryInput);
        auto tLevelSet = LevelSetTopology{tInput};
        return library::FactoryTypes{make_level_set_geometry(tInput), tLevelSet.initialGuess(), tLevelSet.bounds(),
                                     make_topology_output(tInput)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kLevelSetTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::level_set_topology>{
        [](const input_parser::level_set_topology& aInput) { return detail::validate_background_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_output_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_lower_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_upper_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_radius(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_spacing(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_bbox(aInput); },
        [](const input_parser::level_set_topology& aInput)
        { return validate_unique_fixed_block_names(aInput, kMeshNameAccessor); },
        [](const input_parser::level_set_topology& aInput)
        { return validate_fixed_block_names_exist(aInput, kMeshNameAccessor); },
        [](const input_parser::level_set_topology& aInput)
        { return validate_at_least_one_design_block(aInput, kMeshNameAccessor); }};

auto row_vector_times_adjoint_jacobian(const linear_algebra::DynamicVector<double>& aDesignParameters,
                                       const mesh::Mesh& aBackgroundMesh,
                                       const std::filesystem::path& aCutMeshPath,
                                       const third_party_integration::krino::VoidPhase aVoidRegion,
                                       const linear_algebra::DynamicVector<double>& aVector,
                                       const double aFixedLevelSetValue)
    -> std::unordered_map<tpik::KrinoGlobalNodeID, stk::math::Vector3d>
{
    const auto tDesignVariableConverter = mesh::DesignVariablesConversion{aBackgroundMesh};

    const auto tBackgroundMeshWithLevelSets = tDesignVariableConverter.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});
    const auto& tLevelSetJacobian = tpik::generate_computational_mesh(tBackgroundMeshWithLevelSets, aFixedLevelSetValue,
                                                                      tpik::CutMeshFilePath{aCutMeshPath}, aVoidRegion);

    const auto tLevelSetSpaceVector = mesh::DesignVariablesConversion{aBackgroundMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aVector.stdVector()});
    return tpik::level_set_row_vector_adjoint_jacobian_product(tLevelSetSpaceVector, tLevelSetJacobian);
}

auto analysis_domain_mesh(const mesh::Mesh& aMesh) -> analysis::AnalysisDomainMesh
{
    const auto tNumberOfMeshNodes = mesh::EntityCounts{aMesh}.numberOfNodes();
    const auto tMeshField = std::vector(tNumberOfMeshNodes, 0.0);
    return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tMeshField});
}

auto analysis_domain_mesh(const std::filesystem::path& aMeshPath) -> analysis::AnalysisDomainMesh
{
    const auto tMesh = mesh::Mesh{aMeshPath};
    return analysis_domain_mesh(tMesh);
}

auto assembled_row_vector_times_adjoint_jacobian(
    const analysis::AnalysisDomainMesh& aCutMeshSpaceVector,
    const std::unordered_map<tpik::KrinoGlobalNodeID, stk::math::Vector3d>& tAdjointJacobianTimesVector)
    -> linear_algebra::DynamicVector<double>
{
    const auto tCutMeshSpaceVectorView = analysis::AnalysisDomainMeshRandomAccessView{aCutMeshSpaceVector};
    auto tFlattenedAdjointJacobianTimesVector = std::vector<double>(kDimensions * tCutMeshSpaceVectorView.size(), 0.0);
    auto tFlattenedAdjointJacobianTimesVectorVertexView =
        utilities::make_multi_vector_view<kDimensions>(tFlattenedAdjointJacobianTimesVector);
    for (const auto& [tGlobalCutMeshIndex, tNodalSensitivity] : tAdjointJacobianTimesVector)
    {
        const auto tCutmeshFieldValue = tCutMeshSpaceVectorView[tGlobalCutMeshIndex];
        assert(tCutmeshFieldValue);

        const auto tIndex = utilities::VectorIndex{tCutmeshFieldValue->mDesignVariableVectorIndex};
        tFlattenedAdjointJacobianTimesVectorVertexView(tIndex, kXComponent) = tNodalSensitivity[kXComponent.mValue];
        tFlattenedAdjointJacobianTimesVectorVertexView(tIndex, kYComponent) = tNodalSensitivity[kYComponent.mValue];
        tFlattenedAdjointJacobianTimesVectorVertexView(tIndex, kZComponent) = tNodalSensitivity[kZComponent.mValue];
    }
    return linear_algebra::DynamicVector<double>{std::move(tFlattenedAdjointJacobianTimesVector)};
}

auto sphere_pattern(const input_parser::level_set_topology& aInput) -> third_party_integration::krino::SpherePatternData
{
    return third_party_integration::krino::SpherePatternData{
        {aInput.sphere_pattern_bbox_min_x.value(), aInput.sphere_pattern_bbox_min_y.value(),
         aInput.sphere_pattern_bbox_min_z.value()},
        {aInput.sphere_pattern_bbox_max_x.value(), aInput.sphere_pattern_bbox_max_y.value(),
         aInput.sphere_pattern_bbox_max_z.value()},
        aInput.sphere_pattern_radius.value(),
        aInput.sphere_pattern_spacing.value()};
}

auto remove_fixed_block_fields(const std::vector<double>& aLevelSetValues, const mesh::Mesh& aBackgroundMesh)
    -> std::vector<double>
{
    const auto tFixedBlockIDs =
        mesh::block_ids(mesh::Mesh{aBackgroundMesh.filePath()}, aBackgroundMesh.fixedBlockOrdinals());
    auto tInitialGuessOnAnalysisDomainMesh =
        mesh::DesignVariablesConversion{mesh::Mesh{aBackgroundMesh.filePath()}}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{aLevelSetValues});
    tInitialGuessOnAnalysisDomainMesh =
        analysis::remove_block_fields(std::move(tInitialGuessOnAnalysisDomainMesh), tFixedBlockIDs);

    return mesh::DesignVariablesConversion{aBackgroundMesh}
        .analysisDomainMeshToNodalFieldVector(tInitialGuessOnAnalysisDomainMesh)
        .mValue;
}

auto void_phase(const input_parser::level_set_topology& aInput)
{
    return aInput.include_void_region.value() ? third_party_integration::krino::VoidPhase::kIncludeInMesh
                                              : third_party_integration::krino::VoidPhase::kExcludeFromMesh;
}

}  // namespace

LevelSetTopology::LevelSetTopology(const input_parser::level_set_topology& aInput)
    : mBackgroundMesh(mesh_from_input(aInput)),
      mCutMesh(utilities::make_filename_unique(kKrinoCutMeshBaseName)),
      mOutputMesh(aInput.output_mesh_name.value().mToken),
      mVoidRegion(void_phase(aInput)),
      mLevelSetLowerBound(aInput.level_set_lower_bound.value()),
      mLevelSetUpperBound(aInput.level_set_upper_bound.value()),
      mLevelSetPrimitives{{}, tpik::generate_spheres(sphere_pattern(aInput))}
{
}

LevelSetTopology::~LevelSetTopology() { std::filesystem::remove(mCutMesh); }

auto LevelSetTopology::bounds() const -> std::pair<std::vector<double>, std::vector<double>>
{
    const unsigned int tNumNodes = mesh::EntityCounts{mBackgroundMesh}.numberOfDesignDomainNodes();
    return {std::vector<double>(tNumNodes, mLevelSetLowerBound), std::vector<double>(tNumNodes, mLevelSetUpperBound)};
}

auto LevelSetTopology::initialGuess() const -> linear_algebra::DynamicVector<double>
{
    auto tLevelSetValues = tpik::initialize_mesh_with_level_set_primitives(
        tpik::BackgroundMeshFilePath{mBackgroundMesh.filePath()}, mLevelSetPrimitives, mVoidRegion);

    if (mBackgroundMesh.fixedBlockOrdinals().empty())
    {
        return linear_algebra::DynamicVector<double>(std::move(tLevelSetValues));
    }

    return linear_algebra::DynamicVector<double>(remove_fixed_block_fields(tLevelSetValues, mBackgroundMesh));
}

auto LevelSetTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> analysis::AnalysisDomainMesh
{
    const auto tAnalysisMesh = mesh::DesignVariablesConversion{mBackgroundMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});

    tpik::generate_computational_mesh(tAnalysisMesh, mLevelSetUpperBound, tpik::CutMeshFilePath{mCutMesh}, mVoidRegion);

    return analysis::AnalysisDomainMesh{mCutMesh, {}};
}

auto LevelSetTopology::jacobian(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> linear_algebra::JacobianMultiplier
{
    return linear_algebra::JacobianMultiplier{
        [this, aDesignParameters](const linear_algebra::DynamicVector<double>& aVector)
        {
            auto tBackgroundMeshWithLevelSetField =
                mesh::DesignVariablesConversion{mBackgroundMesh}.nodalFieldToAnalysisDomainMesh(
                    mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});

            const auto& tLevelSetJacobian = tpik::generate_computational_mesh(
                tBackgroundMeshWithLevelSetField, mLevelSetUpperBound, tpik::CutMeshFilePath{mCutMesh}, mVoidRegion);

            const auto tCutMeshSpaceVector = analysis_domain_mesh(mCutMesh);

            const auto tVectorJacobianProduct =
                tpik::level_set_row_vector_jacobian_product(aVector.stdVector(), tCutMeshSpaceVector, tLevelSetJacobian,
                                                            std::move(tBackgroundMeshWithLevelSetField));

            const auto tVectorJacobianProductView = analysis::AnalysisDomainMeshSequentialView{tVectorJacobianProduct};
            auto tResultVector = std::vector<double>(tVectorJacobianProductView.size(), 0.0);
            for (const auto& tBackgroundInfoProxy : tVectorJacobianProductView)
            {
                const auto& tBackgroundInfo = static_cast<const analysis::ScalarFieldValue&>(tBackgroundInfoProxy);
                tResultVector[tBackgroundInfo.mDesignVariableVectorIndex] = tBackgroundInfo.mValue;
            }
            return linear_algebra::DynamicVector<double>{std::move(tResultVector)};
        }};
}

auto LevelSetTopology::adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> linear_algebra::AdjointJacobianMultiplier
{
    return linear_algebra::AdjointJacobianMultiplier{
        linear_algebra::JacobianMultiplier{
            [this, aDesignParameters](const linear_algebra::DynamicVector<double>& aVector)
            {
                const auto tAdjointJacobianTimesVector = row_vector_times_adjoint_jacobian(
                    aDesignParameters, mBackgroundMesh, mCutMesh, mVoidRegion, aVector, mLevelSetUpperBound);
                const auto tCutMeshSpaceVector = analysis_domain_mesh(mCutMesh);
                return assembled_row_vector_times_adjoint_jacobian(tCutMeshSpaceVector, tAdjointJacobianTimesVector);
            }}  // namespace plato::geometry::extension
    };
}

void LevelSetTopology::output(const input_parser::level_set_topology& aInput,
                              const filter::library::FilterFunction& aFilterFunction,
                              const linear_algebra::DynamicVector<double>& aSolution)
{
    const auto tMesh = mesh_from_input(aInput);
    const auto tNodalDesignParameters = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aSolution.stdVector()});

    if (boost::mpi::communicator{}.rank() == 0)
    {
        mesh::MeshFieldWriter{tMesh}.writeAnalysisDomainMesh(restart_file_name(aInput), tNodalDesignParameters,
                                                             kTopologyFieldName, kLevelSetFixedValue);

        mesh::MeshFieldWriter{tMesh}.writeAnalysisDomainMesh(
            filtered_output_file_name(aInput),
            aFilterFunction.evaluate<core::evaluation::kFunction>(tNodalDesignParameters), kTopologyFieldName,
            kLevelSetFixedValue);

        tpik::generate_computational_mesh(tNodalDesignParameters, aInput.level_set_upper_bound.value(),
                                          tpik::CutMeshFilePath{aInput.output_mesh_name->mToken}, void_phase(aInput));
    }
}

auto LevelSetTopology::backgroundMesh() const -> const mesh::Mesh& { return mBackgroundMesh; }

auto make_level_set_geometry(const std::shared_ptr<LevelSetTopology>& aLevelSetTopology,
                             const filter::library::FilterFunction& aFilterFunction) -> library::GeometryFunction
{
    auto tGeometryFunction = library::GeometryFunction{
        [aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return aLevelSetTopology->generateMesh(x); },
        [aLevelSetTopology](const linear_algebra::DynamicVector<double>& x) { return aLevelSetTopology->jacobian(x); },
        [aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return linear_algebra::AdjointJacobianMultiplier{aLevelSetTopology->adjointJacobian(x)}; }};

    const auto tAdaptedFilter = library::adapt_filter(aFilterFunction, aLevelSetTopology->backgroundMesh());
    return core::compose(tGeometryFunction, tAdaptedFilter);
}

auto filtered_output_file_name(const input_parser::level_set_topology& aInput) -> std::filesystem::path
{
    constexpr auto tFilteredFileNamePrefix = std::string_view{"filtered_field_"};
    return std::filesystem::path{std::string{tFilteredFileNamePrefix} + aInput.output_mesh_name->mToken};
}

auto restart_file_name(const input_parser::level_set_topology& aInput) -> std::filesystem::path
{
    constexpr auto tRestartFileNamePrefix = std::string_view{"restart_"};
    return std::filesystem::path{std::string{tRestartFileNamePrefix} + aInput.output_mesh_name->mToken};
}

namespace detail
{
std::optional<std::string> validate_output_mesh_name(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::level_set_topology>(),
                                                   aInput.output_mesh_name, "output_name");
}

std::optional<std::string> validate_background_mesh_name(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::level_set_topology>(),
                                                   aInput.background_mesh_name, "background_mesh_name");
}

std::optional<std::string> validate_lower_bound(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::level_set_topology>(),
                                                           aInput.level_set_lower_bound, "level_set_lower_bound",
                                                           utilities::upper_bounded(utilities::Exclusive{0.0}));
}

std::optional<std::string> validate_upper_bound(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::level_set_topology>(),
                                                           aInput.level_set_upper_bound, "level_set_upper_bound",
                                                           utilities::lower_bounded(utilities::Exclusive{0.0}));
}

std::optional<std::string> validate_sphere_pattern_spacing(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::level_set_topology>(),
                                                           aInput.sphere_pattern_spacing, "sphere_pattern_spacing",
                                                           utilities::lower_bounded(utilities::Exclusive{1e-5}));
}

std::optional<std::string> validate_sphere_pattern_radius(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::level_set_topology>(),
                                                           aInput.sphere_pattern_radius, "sphere_pattern_radius",
                                                           utilities::lower_bounded(utilities::Exclusive{1e-5}));
}

std::optional<std::string> validate_sphere_pattern_bbox(const input_parser::level_set_topology& aInput)
{
    if (aInput.sphere_pattern_bbox_max_x.value() < aInput.sphere_pattern_bbox_min_x.value() ||
        aInput.sphere_pattern_bbox_max_y.value() < aInput.sphere_pattern_bbox_min_y.value() ||
        aInput.sphere_pattern_bbox_max_z.value() < aInput.sphere_pattern_bbox_min_z.value())
    {
        return std::string("Invalid sphere pattern bounding box was specified.");
    }
    return std::nullopt;
}

}  // namespace detail

}  // namespace plato::geometry::extension
