#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <string_view>

#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kMassAppName = std::string_view{"test-mass-app"};

input_parser::objective valid_mass_objective_input(const boost::mpi::communicator& aComm)
{
    auto tInput = input_parser::objective{};
    tInput.app = input_parser::AppName{std::string{kMassAppName}};
    tInput.criterion = input_parser::CriterionName{"mass"};
    tInput.number_of_processors = aComm.size();
    tInput.aggregation_weight = 42.0;
    return tInput;
}
}  // namespace

TEST(CriterionFactory, ValidObjective)
{
    namespace ptu = plato::test_utilities;

    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tComm);
    tComm.barrier();

    const auto tInput = valid_mass_objective_input(tComm) | test_utilities::create_valid_brick_shape_geometry() |
                        test_utilities::create_valid_example_rol_optimization();

    const auto tData = process_manager::library::make_validated_input(tInput);

    EXPECT_GT(tComm.size(), 1);
    ASSERT_FALSE(tData.objectives().rawInput().empty());
    EXPECT_NO_THROW([[maybe_unused]] auto tFunction =
                        criteria::library::make_criterion_function(tData.objectives().rawInput().front(), tComm));
}
}  // namespace plato::integration_tests::parallel
