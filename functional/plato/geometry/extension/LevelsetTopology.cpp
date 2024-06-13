#include "plato/geometry/extension/LevelsetTopology.hpp"

#include <PlatoKrinoInterface.hpp>

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::geometry::extension
{

namespace
{

std::map<unsigned int, double> calculateDFDLS(
    const std::map<unsigned int, stk::math::Vector3d>& aDFDXMap,
    const std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP>& aDXDP,
    const std::vector<unsigned int>& aBackgroundNodemap)
{
    std::map<unsigned int, double> tDFDLS;
    for (auto tNodeID : aBackgroundNodemap)
    {
        tDFDLS[tNodeID] = 0.0;
    }
    std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP>::const_iterator tDXDPMapIter = aDXDP.begin();
    while (tDXDPMapIter != aDXDP.end())
    {
        const unsigned int tCurInterfaceNodeID = tDXDPMapIter->first;
        if (aDFDXMap.count(tCurInterfaceNodeID) == 0)
        {
            std::cout << "ERROR: Cut mesh interface global node id does not have a corresponding DFDX entry!"
                      << std::endl;
            throw 1;
        }

        for (size_t j = 0; j < tDXDPMapIter->second.parentNodeIds.size(); ++j)
        {
            const unsigned int tCurBackgroundMeshNodeID = tDXDPMapIter->second.parentNodeIds[j];
            double tContribution = 0.0;
            for (size_t w = 0; w < 3; ++w)
            {
                tContribution += aDFDXMap.at(tCurInterfaceNodeID)[w] * tDXDPMapIter->second.parentDXDP[j][w];
            }
            tDFDLS[tCurBackgroundMeshNodeID] += tContribution;
        }
        tDXDPMapIter++;
    }
    return tDFDLS;
}

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
        [](const input_parser::levelset_topology& aInput) { return detail::validate_mesh_name(aInput); },
        [](const input_parser::levelset_topology& aInput) { return detail::validate_output_name(aInput); }};
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
    tPlatoKrinoInterface.cut_mesh_and_return_sensitivities(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(),
                                                           mIncludeVoidRegion);
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
            static int tIndex = 1;
            std::string tFilename = "DFDX" + std::to_string(tIndex) + ".txt";
            std::ofstream tFile;
            tFile.open(tFilename);
            for (auto g : x.stdVector())
            {
                tFile << g << std::endl;
            }
            tFile.close();

            Plato::Krino::PlatoKrinoInterface tPlatoKrinoInterface;
            const std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP> tDXDP =
                tPlatoKrinoInterface.cut_mesh_and_return_sensitivities(
                    mBackgroundMesh, mCutMesh, aDesignParameters.stdVector(), mIncludeVoidRegion);
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

            tFilename = "FullGradient" + std::to_string(tIndex) + ".txt";
            tFile.open(tFilename);
            for (auto g : tDFDLSVector)
            {
                tFile << g << std::endl;
            }
            tFile.close();
            tIndex++;

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
std::optional<std::string> validate_output_name(const input_parser::levelset_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::levelset_topology>(),
                                                   aInput.output_mesh_name, "output_name");
}

template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_mesh_name(const Geometry& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<Geometry>(), aInput.background_mesh_name,
                                                   "background_mesh_name");
}

}  // namespace detail

}  // namespace plato::geometry::extension
