#ifndef PLATO_GEOMETRY_EXTENSION_CUBIT_TESTUTILITIES_CUBITTESTFIXTURE
#define PLATO_GEOMETRY_EXTENSION_CUBIT_TESTUTILITIES_CUBITTESTFIXTURE

#include <gtest/gtest.h>

#include <filesystem>

#include "plato/geometry/extension/cubit/CubitGeometry.hpp"

namespace plato::geometry::extension::cubit::test_utilities
{

///@brief A simple test fixture for cubit that generates a block that is parameterized by a height, scale and a shift.
/// This can be used in parallel or serial test.
///
class CubitTestFixture : public ::testing::Test
{
   public:
    ///@brief Construct a cubit test fixture and create the journal and mesh file the check functions rely on.
    CubitTestFixture();

    ///@brief Destructor cleans up the journal file, mesh file, and log file.
    ~CubitTestFixture();

    ///@brief Compute the jacobian and compare against gold values
    void checkJacobian();

    ///@brief Compute the adjoint jacobian and compare against gold values
    void checkAdjointJacobian();

   protected:
    CubitGeometry mCubitGeometry;
    std::filesystem::path mMeshName;
};

///@brief Helper function for test fixture that creates a valid input struct for this example
[[nodiscard]] auto create_cubit_input_for_test_fixture() -> plato::input_parser::cubit_parameterized_shape;

///@brief Helper function that creates the journal file needed for this example
[[nodiscard]] auto create_meshing_journal_file() -> std::filesystem::path;

}  // namespace plato::geometry::extension::cubit::test_utilities

#endif
