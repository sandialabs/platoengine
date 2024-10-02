#include "plato/geometry/extension/LevelsetTopology.hpp"

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/third_party_integration/krino/Interface.hpp"
#include "plato/third_party_integration/krino/SphereBuilder.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/ParameterBounds.hpp"

using namespace plato::third_party_integration::krino;

namespace plato::geometry::extension
{

namespace
{

constexpr double kLevelsetFixedValue = 1.0;
constexpr auto kTopologyFieldName = std::string_view{"Topology"};

std::function<void(const linear_algebra::DynamicVector<double>&)> make_topology_output(
    const std::filesystem::path& aInputMeshName, const std::filesystem::path& aOutputMeshName)
{
    return [aInputMeshName, aOutputMeshName](const linear_algebra::DynamicVector<double>& aSolution)
    { return LevelsetTopology::output(aInputMeshName, aSolution, aOutputMeshName); };
}

void initialize_krino()
{
    static bool tIsInitialized = false;
    if (!tIsInitialized)
    {
        initialize_environment_for_krino(MPI_COMM_WORLD);
    }
}

/// Static registration for library
[[maybe_unused]] static auto kLevelsetTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::levelset_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        initialize_krino();
        const auto& tInput = core::validated_variant_raw_input<input_parser::levelset_topology>(aGeometryInput);
        auto tLevelset = LevelsetTopology{tInput};
        return library::FactoryTypes{
            make_topology_geometry(tLevelset), tLevelset.initialGuess(tInput.background_mesh_name.value().mToken),
            tLevelset.bounds(tInput.background_mesh_name.value().mToken),
            make_topology_output(tInput.background_mesh_name.value().mToken, tInput.output_mesh_name.value().mToken)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kLevelsetTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::levelset_topology>{
        [](const input_parser::levelset_topology& aInput) { return detail::validate_background_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_cut_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_output_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_lower_bound(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_upper_bound(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_radius(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_spacing(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_bbox(aInput); }};
}  // namespace

LevelsetTopology::LevelsetTopology(const input_parser::levelset_topology& aInput)
    : mBackgroundMesh(aInput.background_mesh_name.value().mToken),
      mCutMesh(aInput.cut_mesh_name.value().mToken),
      mOutputMesh(aInput.output_mesh_name.value().mToken),
      mIncludeVoidRegion(aInput.include_void_region.value()),
      mLevelsetLowerBound(aInput.levelset_lower_bound.value()),
      mLevelsetUpperBound(aInput.levelset_upper_bound.value()),
      mNumDesignParameters(mesh::EntityCounts{mesh::Mesh{mBackgroundMesh}}.numberOfNodes()),
      mSpherePattern({{aInput.sphere_pattern_bbox_min_x.value(), aInput.sphere_pattern_bbox_min_y.value(),
                       aInput.sphere_pattern_bbox_min_z.value()},
                      {aInput.sphere_pattern_bbox_max_x.value(), aInput.sphere_pattern_bbox_max_y.value(),
                       aInput.sphere_pattern_bbox_max_z.value()},
                      aInput.sphere_pattern_radius.value(),
                      aInput.sphere_pattern_spacing.value()})
{
    generateLevelsetInitializationPrimitives();
}

void LevelsetTopology::generateLevelsetInitializationPrimitives()
{
    const std::vector<Sphere> tSpheres = generate_spheres(mSpherePattern);
    mLevelsetPrimitives.mSpheres.insert(mLevelsetPrimitives.mSpheres.end(), tSpheres.begin(), tSpheres.end());
}

std::pair<std::vector<double>, std::vector<double>> LevelsetTopology::bounds(
    const std::filesystem::path& aMeshFileName) const
{
    const unsigned int tNumNodes = mesh::EntityCounts{mesh::Mesh{aMeshFileName}}.numberOfNodes();
    return {std::vector<double>(tNumNodes, mLevelsetLowerBound), std::vector<double>(tNumNodes, mLevelsetUpperBound)};
}

linear_algebra::DynamicVector<double> LevelsetTopology::initialGuess(const std::filesystem::path& aMeshFileName) const
{
    std::vector<double> tCurLevelsetValues =
        initialize_mesh_with_levelset_primitives(aMeshFileName, mCutMesh, mLevelsetPrimitives, mIncludeVoidRegion);
    return linear_algebra::DynamicVector<double>(tCurLevelsetValues);
}

analysis::AnalysisDomainMesh LevelsetTopology::generateMesh(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
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
    std::ignore =
        generate_computational_mesh(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(), mIncludeVoidRegion);
    return analysis::AnalysisDomainMesh{mCutMesh, {}};
}

linear_algebra::JacobianMultiplier LevelsetTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    return linear_algebra::JacobianMultiplier{
        mNumDesignParameters, [this, &aDesignParameters](const linear_algebra::DynamicVector<double>& x)
        {
            /* When introducing filtering do the following:
            return DFDLS * mFilter.df(tAnalysisDomainMesh); where tAnalysisDomainMesh corresponds to the
            background mesh that the design variables live on.  This can be created with a DesignVariableConverter (see
            DensityToplogy::jacobian() for example)
            */
            const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> tGlobalIDToDXDP =
                generate_computational_mesh(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(),
                                            mIncludeVoidRegion);
            const std::vector<unsigned int> tCutNodeMap = mesh::EntityRetrieval{mesh::Mesh{mCutMesh}}.globalNodeIds();
            const std::vector<unsigned int> tBackgroundNodeMap =
                mesh::EntityRetrieval{mesh::Mesh{mBackgroundMesh}}.globalNodeIds();
            ;
            const std::unordered_map<unsigned int, stk::math::Vector3d> tGlobalIDToDFDXMap =
                assemble_global_id_to_dfdx_map(x.stdVector(), tCutNodeMap, DFDXFormatting::OneToN);
            const std::unordered_map<unsigned int, double> tDFDLS =
                calculate_dfdls(tGlobalIDToDFDXMap, tGlobalIDToDXDP, tBackgroundNodeMap);
            std::vector<double> tDFDLSVector(tDFDLS.size());

            for (const auto& [tIndex, tBackgroundNodeID] : utilities::enumerate(tBackgroundNodeMap))
            {
                tDFDLSVector[tIndex] = tDFDLS.at(tBackgroundNodeID);
            }
            return linear_algebra::DynamicVector<double>{tDFDLSVector};
        }};
}

void LevelsetTopology::output(const std::filesystem::path& aInputMeshName,
                              const linear_algebra::DynamicVector<double>& aSolution,
                              const std::filesystem::path& aOutputMeshName)
{
    const auto tMesh = mesh::Mesh{aInputMeshName};
    const auto tNodalDesignParameters = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aSolution.stdVector()});
    mesh::MeshFieldWriter{tMesh}.writeAnalysisDomainMesh(aOutputMeshName, tNodalDesignParameters, kTopologyFieldName,
                                                         kLevelsetFixedValue);
}

auto make_topology_geometry(const LevelsetTopology& aLevelsetTopology)
    -> core::Function<analysis::AnalysisDomainMesh,
                      linear_algebra::JacobianMultiplier,
                      const linear_algebra::DynamicVector<double>&>
{
    return core::make_function([tLevelsetTopology = aLevelsetTopology](const linear_algebra::DynamicVector<double>& x)
                               { return tLevelsetTopology.generateMesh(x); },
                               [tLevelsetTopology = aLevelsetTopology](const linear_algebra::DynamicVector<double>& x)
                               { return tLevelsetTopology.jacobian(x); });
}

namespace detail
{
std::optional<std::string> validate_output_mesh_name(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::levelset_topology>(),
                                                   aInput.output_mesh_name, "output_name");
}

std::optional<std::string> validate_background_mesh_name(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::levelset_topology>(),
                                                   aInput.background_mesh_name, "background_mesh_name");
}

std::optional<std::string> validate_cut_mesh_name(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::levelset_topology>(),
                                                   aInput.cut_mesh_name, "cut_mesh_name");
}

std::optional<std::string> validate_lower_bound(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.levelset_lower_bound, "levelset_lower_bound",
                                                           utilities::upper_bounded(utilities::Exclusive{0.0}));
}

std::optional<std::string> validate_upper_bound(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.levelset_upper_bound, "levelset_upper_bound",
                                                           utilities::lower_bounded(utilities::Exclusive{0.0}));
}

std::optional<std::string> validate_sphere_pattern_spacing(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.sphere_pattern_spacing, "sphere_pattern_spacing",
                                                           utilities::lower_bounded(utilities::Exclusive{1e-5}));
}

std::optional<std::string> validate_sphere_pattern_radius(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.sphere_pattern_radius, "sphere_pattern_radius",
                                                           utilities::lower_bounded(utilities::Exclusive{1e-5}));
}

std::optional<std::string> validate_sphere_pattern_bbox(const input_parser::levelset_topology& aInput)
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
