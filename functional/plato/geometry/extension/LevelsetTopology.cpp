#include "plato/geometry/extension/LevelsetTopology.hpp"

#include <PlatoKrinoInterface.hpp>

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::geometry::extension
{

namespace
{
constexpr double kInitialLevelsetValue = 1.0;
constexpr double kLevelsetLowerBound = -100.0;
constexpr double kLevelsetUpperBound = 100.0;

std::map<unsigned int, double> calculateDFDLS(
    const std::map<unsigned int, stk::math::Vector3d>& aDFDXMap,
    const std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP>& aDXDP)
{
    std::map<unsigned int, double> tDFDLS;
    std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP>::const_iterator tDXDPMapIter = aDXDP.begin();
    while (tDXDPMapIter != aDXDP.end())
    {
        unsigned int tCurInterfaceNodeID = tDXDPMapIter->first;
        if (aDFDXMap.count(tCurInterfaceNodeID) == 0)
        {
            std::cout << "ERROR: Cut mesh interface global node id does not have a corresponding DFDX entry!"
                      << std::endl;
            throw 1;
        }

        for (size_t j = 0; j < tDXDPMapIter->second.parentNodeIds.size(); ++j)
        {
            unsigned int tCurBackgroundMeshNodeID = tDXDPMapIter->second.parentNodeIds[j];
            double tContribution = 0.0;
            for (size_t w = 0; w < 3; ++w)
            {
                tContribution += aDFDXMap.at(tCurInterfaceNodeID)[w] * tDXDPMapIter->second.parentDXDP[j][w];
            }
            if (tDFDLS.count(tCurBackgroundMeshNodeID) == 0)
            {
                tDFDLS[tCurBackgroundMeshNodeID] = tContribution;
            }
            else
            {
                tDFDLS[tCurBackgroundMeshNodeID] += tContribution;
            }
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

/// Static registration for library
[[maybe_unused]] static auto kLevelsetTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::levelset_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
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
    std::vector<Plato::Krino::Sphere> tSpheres = Plato::Krino::generateSpheres(mSpherePattern);
    mLevelsetPrimitives.mSpheres.insert(mLevelsetPrimitives.mSpheres.end(), tSpheres.begin(), tSpheres.end());
    ;
}

std::pair<std::vector<double>, std::vector<double>> LevelsetTopology::bounds(const std::filesystem::path& aMeshFileName)
{
    const unsigned int tNumNodes = plato::utilities::read_mesh_node_size(aMeshFileName);
    return {std::vector<double>(tNumNodes, kLevelsetLowerBound), std::vector<double>(tNumNodes, kLevelsetUpperBound)};
}

linear_algebra::DynamicVector<double> LevelsetTopology::initialGuess(const std::filesystem::path& aMeshFileName)
{
    Plato::Krino::PlatoKrinoInterface tPlatoKrinoInterface;
    std::vector<double> tCurLevelsetValues =
        tPlatoKrinoInterface.initialize_mesh_with_levelset_primitives_and_return_levelset_values(
            aMeshFileName, mCutMesh, mLevelsetPrimitives);
    return linear_algebra::DynamicVector<double>(tCurLevelsetValues);
}

core::MeshProxy LevelsetTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    Plato::Krino::PlatoKrinoInterface tPlatoKrinoInterface;
    tPlatoKrinoInterface.cut_mesh_and_return_sensitivities(mBackgroundMesh, mCutMesh, aDesignParameters.stdVector());
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
            std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP> tDXDP =
                tPlatoKrinoInterface.cut_mesh_and_return_sensitivities(mBackgroundMesh, mCutMesh,
                                                                       aDesignParameters.stdVector());
            const std::vector<unsigned int> tGlobalNodeIDs = utilities::extract_global_node_ids(mCutMesh);
            std::map<unsigned int, stk::math::Vector3d> tDFDXMap = Plato::Krino::assembleGlobalIDToDFDXMap(
                x.stdVector(), tGlobalNodeIDs, Plato::Krino::DFDXFormatting::OneToN);
            std::map<unsigned int, double> tDFDLS = calculateDFDLS(tDFDXMap, tDXDP);
            std::vector<double> tDFDLSVector(tDFDLS.size());
            std::map<unsigned int, double>::iterator it = tDFDLS.begin();
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
