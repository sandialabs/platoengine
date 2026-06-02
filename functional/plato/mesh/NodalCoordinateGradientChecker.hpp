#ifndef PLATO_MESH_NODALCOORDINATEGRADIENTCHECKER
#define PLATO_MESH_NODALCOORDINATEGRADIENTCHECKER

#include <boost/filesystem.hpp>
#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/string.hpp>
#include <cassert>
#include <filesystem>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/CoordinateUtilities.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/MutableCoordinateMesh.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/GradientChecker.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::mesh
{
/// @brief Creates a GradientChecker, translating function arguments from a vector to a mesh's nodal coordinates.
/// @pre @a aBaseMesh must be a valid mesh on disk.
[[nodiscard]] auto make_nodal_coordinate_gradient_checker(auto aF,
                                                          auto aDf,
                                                          const std::filesystem::path& aBaseMesh,
                                                          const boost::mpi::communicator& aCommunicator)
{
    assert(std::filesystem::exists(aBaseMesh));
    const auto tComputeFunction =
        [aCommunicator, aBaseMesh](const auto& aFunction, const linear_algebra::DynamicVector<double>& aCoordinates)

    {
        constexpr auto tRootRank = 0;
        std::string tPerturbedMeshFile = {};
        plato::utilities::execute_on_root(
            aCommunicator,
            [&aCoordinates, &aBaseMesh, &tPerturbedMeshFile]()
            {
                assert(std::filesystem::exists(aBaseMesh));
                const auto tMesh = Mesh{aBaseMesh};
                auto tMutableCoordinateMesh = MutableCoordinateMesh{tMesh};
                tMutableCoordinateMesh.updateNodalCoordinates(
                    mesh::dynamic_vector_to_nodal_coordinates(aCoordinates, EntityCounts{tMesh}.spatialDimensions()));
                tPerturbedMeshFile = plato::utilities::make_filename_unique(aBaseMesh).string();
                tMutableCoordinateMesh.writeMeshToDisk(tPerturbedMeshFile);
            });
        boost::mpi::broadcast(aCommunicator, tPerturbedMeshFile, tRootRank);
        assert(std::filesystem::exists(tPerturbedMeshFile));
        auto tResult = aFunction(tPerturbedMeshFile);
        plato::utilities::execute_on_root(aCommunicator,
                                          [tPerturbedMeshFile]() { std::filesystem::remove(tPerturbedMeshFile); });
        return tResult;
    };

    return plato::utilities::GradientChecker{
        [tComputeFunction, mF = std::move(aF)](const linear_algebra::DynamicVector<double>& aCoordinates)
        { return tComputeFunction(mF, aCoordinates); },
        [tComputeFunction, mDf = std::move(aDf)](const linear_algebra::DynamicVector<double>& aCoordinates,
                                                 const linear_algebra::DynamicVector<double>& aDirection)
        { return aDirection.dot(tComputeFunction(mDf, aCoordinates)); }};
}
}  // namespace plato::mesh

#endif
