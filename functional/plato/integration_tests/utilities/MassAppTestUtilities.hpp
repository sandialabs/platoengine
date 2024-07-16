#ifndef PLATO_INTEGRATION_TESTS_UTILITIES
#define PLATO_INTEGRATION_TESTS_UTILITIES

#include <boost/mpi/communicator.hpp>
#include <string>
#include <string_view>
#include <utility>

#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::integration_tests::utilities
{
/// @brief Registers a plugin for the MassApp test shared library and returns the registered name and the number of apps
/// successfully registered.
/// @note This assumes that the actual shared library is in the current working directory, which
///  will be the case for any unit tests run from the build directory.
[[nodiscard]] test_utilities::TestDirectorySetupTeardown register_test_mass_app(
    std::string_view aAppName, const boost::mpi::communicator& aComm = boost::mpi::communicator{});

/// @brief Creates test input that has a brick shape geometry, ROL optimization, and an objective
/// with name @a aMassAppName and number of processors @a aNumProcessors.
[[nodiscard]] process_manager::library::ValidatedInput create_test_mass_app_input(
    const input_parser::AppName& aMassAppName,
    const input_parser::CriterionName& aCriterionName,
    unsigned int aNumProcessors);

/// @brief Creates arbitrary test controls with the associated total volume for a BrickShapeGeometry.
[[nodiscard]] std::pair<linear_algebra::DynamicVector<double>, double> brick_shape_geometry_controls_with_volume();

/// @brief Function that tests registration, loading, and running of the test mass app shared library.
void register_load_run_test(const boost::mpi::communicator& aComm, const test_utilities::TestContext& aTestContext);

}  // namespace plato::integration_tests::utilities

#endif
