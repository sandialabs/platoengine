#include "plato/geometry/extension/LevelsetTopology.hpp"

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/krino_integration/PlatoKrinoInterface.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/ParameterBounds.hpp"
#include "plato/utilities/STKUtilities.hpp"

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
        Plato::Krino::initializeSTKEnvironment(MPI_COMM_WORLD);
        Plato::Krino::initializeKrinoLogging();
    }
}

/// Static registration for library
[[maybe_unused]] static auto kLevelsetTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::levelset_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        initialize_krino();
        const auto& tInput = library::geometry_raw_input<input_parser::levelset_topology>(aGeometryInput);
        auto tLevelset = LevelsetTopology{tInput};
        return library::FactoryTypes{
            make_topology_geometry(tLevelset), tLevelset.initialGuess(tInput.background_mesh_name.value().mName),
            tLevelset.bounds(tInput.background_mesh_name.value().mName),
            make_topology_output(tInput.background_mesh_name.value().mName, tInput.output_mesh_name.value().mName)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kLevelsetTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::levelset_topology>{
        [](const input_parser::levelset_topology& aInput) { return detail::validate_background_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_cut_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_output_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_lower_bound(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_upper_bound(aInput); }};
}  // namespace

LevelsetTopology::LevelsetTopology(const input_parser::levelset_topology& aInput)
    : mBackgroundMesh(aInput.background_mesh_name.value().mName),
      mCutMesh(aInput.cut_mesh_name.value().mName),
      mOutputMesh(aInput.output_mesh_name.value().mName),
      mIncludeVoidRegion(aInput.include_void_region.value()),
      mLevelsetLowerBound(aInput.levelset_lower_bound.value()),
      mLevelsetUpperBound(aInput.levelset_upper_bound.value()),
      mNumDesignParameters(plato::utilities::read_mesh_node_size(mBackgroundMesh)),
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
    const std::vector<Plato::Krino::Sphere> tSpheres = Plato::Krino::generateSpheres(mSpherePattern);
    mLevelsetPrimitives.mSpheres.insert(mLevelsetPrimitives.mSpheres.end(), tSpheres.begin(), tSpheres.end());
}

std::pair<std::vector<double>, std::vector<double>> LevelsetTopology::bounds(
    const std::filesystem::path& aMeshFileName) const
{
    const unsigned int tNumNodes = plato::utilities::read_mesh_node_size(aMeshFileName);
    return {std::vector<double>(tNumNodes, mLevelsetLowerBound), std::vector<double>(tNumNodes, mLevelsetUpperBound)};
}

linear_algebra::DynamicVector<double> LevelsetTopology::initialGuess(const std::filesystem::path& aMeshFileName) const
{
    Plato::Krino::PlatoKrinoInterface tPlatoKrinoInterface;
    std::vector<double> tCurLevelsetValues =
        tPlatoKrinoInterface.initialize_mesh_with_levelset_primitives_and_return_levelset_values(
            aMeshFileName, mCutMesh, mLevelsetPrimitives, mIncludeVoidRegion);
    return linear_algebra::DynamicVector<double>(tCurLevelsetValues);
}

core::MeshProxy LevelsetTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    Plato::Krino::PlatoKrinoInterface tPlatoKrinoInterface;
    tPlatoKrinoInterface.cut_mesh(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(), mIncludeVoidRegion);
    return core::MeshProxy{mCutMesh, std::vector<double>{}};
}

linear_algebra::JacobianMultiplier LevelsetTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    return linear_algebra::JacobianMultiplier{
        /*.mNumColumns=*/mNumDesignParameters,
        /*.mJacobianTimesVectorFunction=*/
        [tMeshProxy = core::MeshProxy{mBackgroundMesh, aDesignParameters.stdVector()}, this,
         &aDesignParameters](const linear_algebra::DynamicVector<double>& x)
        {
            Plato::Krino::PlatoKrinoInterface tPlatoKrinoInterface;
            tPlatoKrinoInterface.cut_mesh(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(), mIncludeVoidRegion);
            const std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP> tDXDP =
                tPlatoKrinoInterface.get_sensitivities();
            const std::vector<unsigned int> tCutNodeMap = utilities::extract_global_node_ids(mCutMesh);
            const std::vector<unsigned int> tBackgroundNodeMap = utilities::extract_global_node_ids(mBackgroundMesh);
            const std::map<unsigned int, stk::math::Vector3d> tDFDXMap = Plato::Krino::assembleGlobalIDToDFDXMap(
                x.stdVector(), tCutNodeMap, Plato::Krino::DFDXFormatting::OneToN);
            const std::map<unsigned int, double> tDFDLS = calculateDFDLS(tDFDXMap, tDXDP, tBackgroundNodeMap);
            std::vector<double> tDFDLSVector(tDFDLS.size());
            std::map<unsigned int, double>::const_iterator it = tDFDLS.begin();
            for (size_t i = 0; i < tDFDLSVector.size(); i++)
            {
                tDFDLSVector[i] = it->second;
                it++;
            }
            linear_algebra::DynamicVector<double> tDFDLSDynVector(tDFDLSVector);
            return tDFDLSDynVector;
        }};
}

void LevelsetTopology::output(const std::filesystem::path& aInputMeshName,
                              const linear_algebra::DynamicVector<double>& aSolution,
                              const std::filesystem::path& aOutputMeshName)
{
    plato::utilities::write_mesh_density(aInputMeshName, aSolution.stdVector(), aOutputMeshName);
}

auto make_topology_geometry(const LevelsetTopology& aLevelsetTopology)
    -> core::Function<core::MeshProxy, linear_algebra::JacobianMultiplier, const linear_algebra::DynamicVector<double>&>
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

}  // namespace detail

}  // namespace plato::geometry::extension
