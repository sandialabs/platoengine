#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <string_view>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kMassAppName = std::string_view{"test-mass-app"};

[[nodiscard]] auto valid_mass_objective_input(const boost::mpi::communicator& aCommunicator) -> input_parser::objective
{
    auto tInput = input_parser::objective{};
    tInput.name = "mass-objective";
    tInput.app = input_parser::AppName{std::string{kMassAppName}};
    tInput.criterion = input_parser::CriterionName{"mass"};
    tInput.number_of_processors = aCommunicator.size();
    constexpr auto tArbitraryAggregationWeight = 42.0;
    tInput.aggregation_weight = tArbitraryAggregationWeight;
    return tInput;
}
}  // namespace

TEST(CriterionFactory, ValidObjective)
{
    namespace pcl = plato::criteria::library;

    const auto tCommunicator = boost::mpi::communicator{};
    const auto tConfigurationTempDirectory = utilities::register_test_mass_app(kMassAppName, tCommunicator);
    tCommunicator.barrier();

    const auto tInput = valid_mass_objective_input(tCommunicator) |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    const auto tData = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tData.hasValue());
    EXPECT_GT(tCommunicator.size(), 1);
    const auto tCriteria = tData.value().get<components::ComponentType::kObjective>().rawInput();
    ASSERT_FALSE(tCriteria.empty());
    EXPECT_NO_THROW([[maybe_unused]] auto tFunction =
                        (pcl::make_criterion_function<pcl::CriterionFunction, input_parser::objective>(
                            tCriteria.front(), tCommunicator)));
}
}  // namespace plato::integration_tests::parallel
