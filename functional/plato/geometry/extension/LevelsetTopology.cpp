#include "plato/geometry/extension/LevelsetTopology.hpp"

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/Interface.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/ParameterBounds.hpp"

using namespace plato::third_party_integration::krino;

namespace plato::geometry::extension
{

namespace
{

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
        initializeSTKEnvironment(MPI_COMM_WORLD);
        initializeKrinoLogging();
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
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_num_in_x(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_num_in_y(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_num_in_z(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_sphere_pattern_radius(aInput); },
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
      mSpherePattern({aInput.sphere_pattern_overlap_bbox.value(),
                      {aInput.sphere_pattern_bbox_min_x.value(), aInput.sphere_pattern_bbox_min_y.value(),
                       aInput.sphere_pattern_bbox_min_z.value()},
                      {aInput.sphere_pattern_bbox_max_x.value(), aInput.sphere_pattern_bbox_max_y.value(),
                       aInput.sphere_pattern_bbox_max_z.value()},
                      {aInput.sphere_pattern_num_x.value(), aInput.sphere_pattern_num_y.value(),
                       aInput.sphere_pattern_num_z.value()},
                      aInput.sphere_pattern_radius.value()})
{
    generateLevelsetInitializationPrimitives();
}

void LevelsetTopology::generateLevelsetInitializationPrimitives()
{
    const std::vector<Sphere> tSpheres = generateSpheres(mSpherePattern);
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
        initializeMeshWithLevelsetPrimitives(aMeshFileName, mCutMesh, mLevelsetPrimitives, mIncludeVoidRegion);
    return linear_algebra::DynamicVector<double>(tCurLevelsetValues);
}

mesh::MeshDesignVariables LevelsetTopology::generateMesh(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    std::ignore =
        generateComputationalMesh(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(), mIncludeVoidRegion);
    return mesh::MeshDesignVariables{mCutMesh, {}};
}

linear_algebra::JacobianMultiplier LevelsetTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    return linear_algebra::JacobianMultiplier{
        mNumDesignParameters, [this, &aDesignParameters](const linear_algebra::DynamicVector<double>& x)
        {
            const std::map<stk::mesh::EntityId, InterfaceNode_DXDP> tGlobalIDToDXDP =
                generateComputationalMesh(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(), mIncludeVoidRegion);
            const std::vector<unsigned int> tCutNodeMap =
                third_party_integration::stk_io::extract_global_node_ids(mCutMesh);
            const std::vector<unsigned int> tBackgroundNodeMap =
                third_party_integration::stk_io::extract_global_node_ids(mBackgroundMesh);
            const std::map<unsigned int, stk::math::Vector3d> tGlobalIDToDFDXMap =
                assembleGlobalIDToDFDXMap(x.stdVector(), tCutNodeMap, DFDXFormatting::OneToN);
            std::map<unsigned int, double> tDFDLS =
                calculateDFDLS(tGlobalIDToDFDXMap, tGlobalIDToDXDP, tBackgroundNodeMap);
            std::vector<double> tDFDLSVector(tDFDLS.size());

            size_t tCounter = 0;
            for (auto tBackgroundNodeID : tBackgroundNodeMap)
            {
                tDFDLSVector[tCounter++] = tDFDLS[tBackgroundNodeID];
            }
            return linear_algebra::DynamicVector<double>{tDFDLSVector};
        }};
}

void LevelsetTopology::output(const std::filesystem::path& aInputMeshName,
                              const linear_algebra::DynamicVector<double>& aSolution,
                              const std::filesystem::path& aOutputMeshName)
{
    const std::vector<unsigned int> tMeshNodeMap =
        third_party_integration::stk_io::extract_global_node_ids(aInputMeshName);
    std::unordered_map<std::size_t, double> aSolutionMap;
    for (size_t i = 0; i < tMeshNodeMap.size(); ++i)
    {
        aSolutionMap[tMeshNodeMap[i]] = aSolution[i];
    }
    third_party_integration::stk_io::write_nodal_density(aInputMeshName, aSolutionMap, aOutputMeshName);
}

auto make_topology_geometry(const LevelsetTopology& aLevelsetTopology)
    -> core::Function<mesh::MeshDesignVariables,
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

std::optional<std::string> validate_sphere_pattern_num_in_x(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.sphere_pattern_num_x, "sphere_pattern_num_x",
                                                           utilities::lower_bounded(utilities::Exclusive{1}));
}

std::optional<std::string> validate_sphere_pattern_num_in_y(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.sphere_pattern_num_y, "sphere_pattern_num_y",
                                                           utilities::lower_bounded(utilities::Exclusive{1}));
}

std::optional<std::string> validate_sphere_pattern_num_in_z(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<input_parser::levelset_topology>(),
                                                           aInput.sphere_pattern_num_z, "sphere_pattern_num_z",
                                                           utilities::lower_bounded(utilities::Exclusive{1}));
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
