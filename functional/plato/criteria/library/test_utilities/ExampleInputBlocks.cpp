#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"

#include "plato/input_parser/InputDefinitions.hpp"

namespace plato::criteria::library::test_utilities
{

auto create_valid_example_constraint_input() -> input_parser::constraint
{
    return input_parser::constraint{/*.name=*/std::string{"bike-shed"},
                                    /*.active=*/true,
                                    /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                                    /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
                                    /*.number_of_processors=*/1U,
                                    /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                    /*.constraint_value=*/0.0,
                                    /*.is_linear=*/true,
                                    /*.constraint_type=*/input_parser::ConstraintTypes::kEqualTo};
}

[[nodiscard]] auto create_valid_example_objective_input() -> input_parser::objective
{
    return input_parser::objective{/*.name=*/std::string{"bike-shed"},
                                   /*.active=*/true,
                                   /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                                   /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
                                   /*.number_of_processors=*/1u,
                                   /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                   /*.aggregation_weight=*/13.0,
                                   /*.objective_goal=*/boost::none};
}
}  // namespace plato::criteria::library::test_utilities
