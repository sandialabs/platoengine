#ifndef PLATO_INTEGRATION_TESTS_UTILITIES
#define PLATO_INTEGRATION_TESTS_UTILITIES

#include <boost/mpi/communicator.hpp>
#include <string>
#include <string_view>
#include <utility>

#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::integration_tests::utilities
{
/// @brief Registers a plugin for the MassApp test shared library and returns the registered name and the number of apps
/// successfully registered.
/// @note This assumes that the actual shared library is in the current working directory, which
///  will be the case for any unit tests run from the build directory.
[[nodiscard]] auto register_test_mass_app(std::string_view aAppName,
                                          const boost::mpi::communicator& aComm = boost::mpi::communicator{})
    -> test_utilities::TestDirectorySetupTeardown;

/// @brief Creates test input that has a brick shape geometry, ROL optimization, and an objective
/// with name @a aMassAppName and number of processors @a aNumProcessors.
[[nodiscard]] auto create_test_mass_app_input(const input_parser::AppName& aMassAppName,
                                              const input_parser::CriterionName& aCriterionName,
                                              unsigned int aNumProcessors) -> input_validation::ValidatedInput;

/// @brief Creates test input that has a brick shape geometry, ROL optimization, ROL constraint check a mass objective,
/// and a vector mass constraint with name @a aMassAppName
[[nodiscard]] auto create_test_mass_vector_constraint_input(const input_parser::AppName& aMassAppName,
                                                            const input_parser::CriterionName& aCriterionName,
                                                            const std::filesystem::path& aMeshName)
    -> input_validation::ValidatedInput;

/// @brief Creates arbitrary test controls with the associated total volume for a BrickShapeGeometry.
[[nodiscard]] auto brick_shape_geometry_controls_with_volume()
    -> std::pair<linear_algebra::DynamicVector<double>, double>;

/// @brief Function that tests registration, loading, and running of the test mass app shared library.
void register_load_run_test(const boost::mpi::communicator& aComm, const test_utilities::TestContext& aTestContext);

/// @brief Function that tests registration of the test mass vector constraint shared library.
void register_load_vector_constraint(const test_utilities::TestContext& aTestContext);

/// @brief The name on disk of the shared library. This can be used to load the library in tests.
[[nodiscard]] constexpr auto mass_app_library_file_name() -> std::string_view;

/// @brief Registers the test mass app shared library and returns a validated input
[[nodiscard]] auto setup_mass_app_for_test(const std::filesystem::path& aMeshFileName)
    -> std::pair<test_utilities::TestDirectorySetupTeardown, input_validation::ValidatedInput>;

constexpr auto mass_app_library_file_name() -> std::string_view { return "libPlatoTestMassCriteria.so"; }
}  // namespace plato::integration_tests::utilities

#endif
