#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <string_view>

#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kMassAppName = std::string_view{"test-mass-app"};

[[nodiscard]] auto valid_mass_objective_input(const boost::mpi::communicator& aComm) -> input_parser::objective
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
    namespace pcl = plato::criteria::library;

    const auto tComm = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tComm);
    tComm.barrier();

    const auto tInput = valid_mass_objective_input(tComm) |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    const auto tData = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tData.hasValue());
    EXPECT_GT(tComm.size(), 1);
    const auto tCriteria = tData.value().get<input_parser::ComponentType::kObjective>().rawInput();
    ASSERT_FALSE(tCriteria.empty());
    EXPECT_NO_THROW(
        [[maybe_unused]] auto tFunction =
            (pcl::make_criterion_function<pcl::CriterionFunction, input_parser::objective>(tCriteria.front(), tComm)));
}
}  // namespace plato::integration_tests::parallel
