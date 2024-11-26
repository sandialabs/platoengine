#include "plato/geometry/extension/LevelSetTopology.hpp"

#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
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

constexpr auto kXComponent = utilities::ComponentIndex{0};
constexpr auto kYComponent = utilities::ComponentIndex{1};
constexpr auto kZComponent = utilities::ComponentIndex{2};

auto make_topology_output(const std::filesystem::path& aInputMeshName, const std::filesystem::path& aOutputMeshName)
    -> std::function<void(const linear_algebra::DynamicVector<double>&)>
{
    return [aInputMeshName, aOutputMeshName](const linear_algebra::DynamicVector<double>& aSolution)
    { return LevelSetTopology::output(aInputMeshName, aSolution, aOutputMeshName); };
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

/// Static registration for library
[[maybe_unused]] static auto kLevelSetTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::level_set_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        initialize_krino();
        const auto& tInput = core::validated_variant_raw_input<input_parser::level_set_topology>(aGeometryInput);
        auto tLevelSet = LevelSetTopology{tInput};
        return library::FactoryTypes{
            make_topology_geometry(tLevelSet), tLevelSet.initialGuess(tInput.background_mesh_name.value().mToken),
            tLevelSet.bounds(tInput.background_mesh_name.value().mToken),
            make_topology_output(tInput.background_mesh_name.value().mToken, tInput.output_mesh_name.value().mToken)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kLevelSetTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::level_set_topology>{
        [](const input_parser::level_set_topology& aInput) { return detail::validate_background_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_cut_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_output_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_lower_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_upper_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_radius(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_spacing(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_bbox(aInput); }};

auto adjoint_jacobian_times_vector(const linear_algebra::DynamicVector<double>& aDesignParameters,
                                   const mesh::Mesh& aBackgroundMesh,
                                   const std::filesystem::path& aCutMeshPath,
                                   const third_party_integration::krino::VoidPhase aVoidRegion,
                                   const linear_algebra::DynamicVector<double>& aVector)
    -> std::unordered_map<tpik::KrinoGlobalNodeID, stk::math::Vector3d>
{
    const auto& tGlobalIDToDXDP = tpik::generate_computational_mesh(
        tpik::BackgroundMeshFilePath{aBackgroundMesh.filePath()}, tpik::CutMeshFilePath{aCutMeshPath},
        aDesignParameters.stdVector(), aVoidRegion);
    const auto tLevelSetSpaceVector = mesh::DesignVariablesConversion{aBackgroundMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aVector.stdVector()});
    return tpik::calculate_adjoint_dfdls(tLevelSetSpaceVector, tGlobalIDToDXDP);
}

auto analysis_domain_mesh(const mesh::Mesh& aMesh) -> analysis::AnalysisDomainMesh
{
    const auto tNumberOfCutMeshNodes = mesh::EntityCounts{aMesh}.numberOfNodes();
    const auto tMeshField = std::vector(tNumberOfCutMeshNodes, 0.0);
    return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tMeshField});
}

auto analysis_domain_mesh(const std::filesystem::path& aMeshPath) -> analysis::AnalysisDomainMesh
{
    const auto tMesh = mesh::Mesh{aMeshPath};
    return analysis_domain_mesh(tMesh);
}

auto assembled_adjoint_jacobian_times_vector(
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

}  // namespace

LevelSetTopology::LevelSetTopology(const input_parser::level_set_topology& aInput)
    : mBackgroundMesh(aInput.background_mesh_name.value().mToken),
      mCutMesh(utilities::make_filename_unique(aInput.cut_mesh_name.value().mToken)),
      mOutputMesh(aInput.output_mesh_name.value().mToken),
      mVoidRegion(aInput.include_void_region.value() ? third_party_integration::krino::VoidPhase::kIncludeInMesh
                                                     : third_party_integration::krino::VoidPhase::kExcludeFromMesh),
      mLevelSetLowerBound(aInput.level_set_lower_bound.value()),
      mLevelSetUpperBound(aInput.level_set_upper_bound.value()),
      mNumDesignParameters(mesh::EntityCounts{mBackgroundMesh}.numberOfNodes()),
      mSpherePattern({{aInput.sphere_pattern_bbox_min_x.value(), aInput.sphere_pattern_bbox_min_y.value(),
                       aInput.sphere_pattern_bbox_min_z.value()},
                      {aInput.sphere_pattern_bbox_max_x.value(), aInput.sphere_pattern_bbox_max_y.value(),
                       aInput.sphere_pattern_bbox_max_z.value()},
                      aInput.sphere_pattern_radius.value(),
                      aInput.sphere_pattern_spacing.value()}),
      mLevelSetPrimitives{{}, tpik::generate_spheres(mSpherePattern)}
{
}
LevelSetTopology::~LevelSetTopology() { std::filesystem::remove(mCutMesh); }

auto LevelSetTopology::bounds(const std::filesystem::path& aMeshFileName) const
    -> std::pair<std::vector<double>, std::vector<double>>
{
    const unsigned int tNumNodes = mesh::EntityCounts{mesh::Mesh{aMeshFileName}}.numberOfNodes();
    return {std::vector<double>(tNumNodes, mLevelSetLowerBound), std::vector<double>(tNumNodes, mLevelSetUpperBound)};
}

auto LevelSetTopology::initialGuess(const std::filesystem::path& aMeshFileName) const
    -> linear_algebra::DynamicVector<double>
{
    auto tCurLevelSetValues = tpik::initialize_mesh_with_level_set_primitives(
        tpik::BackgroundMeshFilePath{aMeshFileName}, tpik::CutMeshFilePath{mCutMesh}, mLevelSetPrimitives, mVoidRegion);
    return linear_algebra::DynamicVector<double>(std::move(tCurLevelSetValues));
}

auto LevelSetTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> analysis::AnalysisDomainMesh
{
    /* When introducing filtering do the following:
    1. Add member variable mMesh that represents the background mesh (See density class).
    2. Add mFilter member variable.
    3. In this function:
        a. Call mFilter.f() on the passed-in design variables.  This will return an AnalysisDomainMesh.
        b. Use mesh::DesignVariableConversion class to convert filtered design variables in the AnalysisDomainMesh
           into a std::vector (see member function for doing this).
        c. Pass the filtered design variables in to generate_computational_mesh().
        d. Keep the same return statement that initializes an AnalysisDomainMesh with the cut mesh
           name and an empty design variable map.
    */
    tpik::generate_computational_mesh(tpik::BackgroundMeshFilePath{mBackgroundMesh.filePath()},
                                      tpik::CutMeshFilePath{mCutMesh}, aDesignParameters.stdVector(), mVoidRegion);
    return analysis::AnalysisDomainMesh{mCutMesh, {}};
}

linear_algebra::JacobianMultiplier LevelSetTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    return linear_algebra::JacobianMultiplier{
        [this, &aDesignParameters](const linear_algebra::DynamicVector<double>& aVector)
        {
            /* When introducing filtering do the following:
            return DFDLS * mFilter.df(tAnalysisDomainMesh); where tAnalysisDomainMesh corresponds to the
            background mesh that the design variables live on.  This can be created with a DesignVariableConverter (see
            DensityToplogy::jacobian() for example)
            */
            const auto& tGlobalIDToDXDP = tpik::generate_computational_mesh(
                tpik::BackgroundMeshFilePath{mBackgroundMesh.filePath()}, tpik::CutMeshFilePath{mCutMesh},
                aDesignParameters.stdVector(), mVoidRegion);
            const auto tCutMeshSpaceVector = analysis_domain_mesh(mCutMesh);

            const auto tBackgroundDFDLS = tpik::calculate_dfdls(aVector.stdVector(), tCutMeshSpaceVector,
                                                                tGlobalIDToDXDP, analysis_domain_mesh(mBackgroundMesh));

            const auto tBackgroundDFDLSView = analysis::AnalysisDomainMeshSequentialView{tBackgroundDFDLS};
            auto tDFDLSVector = std::vector<double>(tBackgroundDFDLSView.size(), 0.0);
            for (const auto& tBackgroundInfoProxy : tBackgroundDFDLSView)
            {
                const auto& tBackgroundInfo = static_cast<const analysis::ScalarFieldValue&>(tBackgroundInfoProxy);
                tDFDLSVector[tBackgroundInfo.mDesignVariableVectorIndex] = tBackgroundInfo.mValue;
            }
            return linear_algebra::DynamicVector<double>{std::move(tDFDLSVector)};
        }};
}

auto LevelSetTopology::adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> linear_algebra::AdjointJacobianMultiplier
{
    return linear_algebra::AdjointJacobianMultiplier{
        linear_algebra::JacobianMultiplier{
            [this, &aDesignParameters](const linear_algebra::DynamicVector<double>& aVector)
            {
                const auto tAdjointJacobianTimesVector =
                    adjoint_jacobian_times_vector(aDesignParameters, mBackgroundMesh, mCutMesh, mVoidRegion, aVector);
                const auto tCutMeshSpaceVector = analysis_domain_mesh(mCutMesh);
                return assembled_adjoint_jacobian_times_vector(tCutMeshSpaceVector, tAdjointJacobianTimesVector);
            }}  // namespace plato::geometry::extension
    };
}

void LevelSetTopology::output(const std::filesystem::path& aInputMeshName,
                              const linear_algebra::DynamicVector<double>& aSolution,
                              const std::filesystem::path& aOutputMeshName)
{
    const auto tMesh = mesh::Mesh{aInputMeshName};
    const auto tNodalDesignParameters = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aSolution.stdVector()});
    mesh::MeshFieldWriter{tMesh}.writeAnalysisDomainMesh(aOutputMeshName, tNodalDesignParameters, kTopologyFieldName,
                                                         kLevelSetFixedValue);
}

auto make_topology_geometry(const LevelSetTopology& aLevelSetTopology) -> library::GeometryFunction
{
    return library::GeometryFunction{
        [tLevelSetTopology = aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return tLevelSetTopology.generateMesh(x); },
        [tLevelSetTopology = aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return tLevelSetTopology.jacobian(x); },
        [tLevelSetTopology = aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return linear_algebra::AdjointJacobianMultiplier{tLevelSetTopology.adjointJacobian(x)}; }};
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

std::optional<std::string> validate_cut_mesh_name(const input_parser::level_set_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::level_set_topology>(),
                                                   aInput.cut_mesh_name, "cut_mesh_name");
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
