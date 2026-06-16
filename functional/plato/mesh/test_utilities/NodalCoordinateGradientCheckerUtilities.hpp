#ifndef PLATO_MESH_TESTUTILITIES_NODALCOORDINATEGRADIENTCHECKERUTILITIES
#define PLATO_MESH_TESTUTILITIES_NODALCOORDINATEGRADIENTCHECKERUTILITIES

#include <boost/mpi.hpp>
#include <filesystem>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::mesh::test_utilities
{

/// @brief Specialized function for running a coordinate based gradient checker for a specific mesh file @a
/// aMeshFilePath and using a simple objective that can be called in serial and parallel unittesters.
void run_coordinate_gradient_check_test(const std::filesystem::path& aMeshFilePath,
                                        const plato::test_utilities::TestContext& aTestContext,
                                        const boost::mpi::communicator& aCommunicator);

/// @brief Function that determines the size of the gradient based on the mesh on disk @a aFilePath
[[nodiscard]] auto gradient_size(const std::filesystem::path& aFilePath) -> std::size_t;

/// @brief Creates a random direction vector sized for the mesh @a aMeshFilePath
[[nodiscard]] auto random_direction(const std::filesystem::path& aMeshFilePath)
    -> linear_algebra::DynamicVector<double>;

/// @brief Retrieve the original coordinates before perturbations for the mesh @a aMeshFilePath
[[nodiscard]] auto original_coordinates(const std::filesystem::path& aMeshFilePath)
    -> linear_algebra::DynamicVector<double>;

}  // namespace plato::mesh::test_utilities

#endif
