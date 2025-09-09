#include "plato/geometry/extension/test_utilities/KrinoWrapperGradientCheckUtilities.hpp"

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::geometry::extension::test_utilities
{
namespace
{
namespace tpik = third_party_integration::krino;
constexpr double kFixedBlockLevelSetValue{1.0};
}  // namespace

auto accumulate_cut_node_coordinates(const std::filesystem::path& aMeshToLoad,
                                     const std::vector<double>& aPerturbedLevelSetField) -> double
{
    const auto tAnalysisDomainMesh =
        mesh::DesignVariablesConversion{mesh::Mesh{aMeshToLoad}}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{aPerturbedLevelSetField});

    const auto tFixedBlocks = std::set<std::string>{};
    const auto tKrino = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0, tFixedBlocks, tpik::SnappingParameters{});
    const auto tCutMesh = std::filesystem::path{"cut_mesh.exo"};
    tKrino.writeCutMesh(tCutMesh, tpik::VoidPhase::kIncludeInMesh);

    auto tCutCoordinates = mesh::EntityRetrieval{mesh::Mesh{tCutMesh}}.nodalCoordinates();

    utilities::execute_on_root(boost::mpi::communicator{}, [&tCutMesh]() { std::filesystem::remove(tCutMesh); });

    std::vector<double> tFlattenedCoordinates;
    tFlattenedCoordinates.reserve(tCutCoordinates.size() * 2U);
    for (const auto& tCutCoordinate : tCutCoordinates)
    {
        tFlattenedCoordinates.push_back(tCutCoordinate.x);
        tFlattenedCoordinates.push_back(tCutCoordinate.y);
    }

    return utilities::pair_wise_accumulate(tFlattenedCoordinates);
}

}  // namespace plato::geometry::extension::test_utilities
