#include "plato/geometry/extension/test_utilities/KrinoWrapperGradientCheckUtilities.hpp"

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::geometry::extension::test_utilities
{
namespace
{
namespace tpik = third_party_integration::krino;
}

auto accumulate_cut_node_coordinates(const std::filesystem::path& aMeshToLoad,
                                     const std::vector<double>& aPerturbedLevelSetField) -> double
{
    const auto tKrino = test_utilities::make_krino_wrapper_from_vector_values(
        aMeshToLoad, test_utilities::InitialLevelSetValues{aPerturbedLevelSetField}, std::nullopt);
    const auto tCutMesh = std::filesystem::path{"cut_mesh.exo"};
    tKrino.writeCutMesh(tCutMesh, tpik::VoidPhase::kIncludeInMesh);

    const auto tOriginalCoordinates = mesh::EntityRetrieval{mesh::Mesh{aMeshToLoad}}.nodalCoordinates();
    auto tCutCoordinates = mesh::EntityRetrieval{mesh::Mesh{tCutMesh}}.nodalCoordinates();

    const auto tCommunicator = boost::mpi::communicator{};
    tCommunicator.barrier();
    if (tCommunicator.rank() == 0)
    {
        std::filesystem::remove(tCutMesh);
    }

    for (const auto& tCoordinate : tOriginalCoordinates)
    {
        const auto tNewEnd = std::remove(tCutCoordinates.begin(), tCutCoordinates.end(), tCoordinate);
        tCutCoordinates.erase(tNewEnd, tCutCoordinates.end());
    }

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
