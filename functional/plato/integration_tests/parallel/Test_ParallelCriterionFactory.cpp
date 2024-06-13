#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <string_view>

#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kCustomAppInput = std::string_view{R"(
          begin objective test1
            app custom_app
            shared_library_path libPlatoTestMassObjective.so
            aggregation_weight 42.0
          end
       )"};
}  // namespace

TEST(CriterionFactory, ValidObjective)
{
    namespace ptu = plato::test_utilities;
    auto tInput = input_parser::parse_input(kCustomAppInput);
    tInput.mBrickShapeGeometry = test_utilities::create_valid_brick_shape_geometry();
    tInput.mROLOptimization = test_utilities::create_valid_example_rol_optimization();

    const auto tComm = boost::mpi::communicator{};
    for (int rank = 1; rank < tComm.size(); ++rank)
    {
        tInput.mObjectives.push_back(test_utilities::create_valid_example_objective());
    }

    const auto tData = process_manager::library::make_validated_input(tInput);

    EXPECT_GT(tComm.size(), 1);
    ASSERT_FALSE(tData.objectives().rawInput().empty());
    EXPECT_NO_THROW([[maybe_unused]] auto tFunction =
                        criteria::library::make_criterion_function(tData.objectives().rawInput().front(), tComm));
}
}  // namespace plato::integration_tests::parallel
