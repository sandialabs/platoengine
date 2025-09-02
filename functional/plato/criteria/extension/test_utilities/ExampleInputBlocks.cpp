#include "plato/criteria/extension/test_utilities/ExampleInputBlocks.hpp"

#include "plato/input_parser/InputDefinitions.hpp"

namespace plato::criteria::extension::test_utilities
{

auto create_valid_example_nodal_sum_objective_input() -> input_parser::objective
{
    return input_parser::objective{/*.name=*/std::string{"nodal_sum"},
                                   /*.active=*/true,
                                   /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                                   /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
                                   /*.number_of_processors=*/1u,
                                   /*.input_files=*/input_parser::FileList{{"my_mesh.exo"}},
                                   /*.aggregation_weight=*/1.0,
                                   /*.objecive_goal=*/boost::none};
}

}  // namespace plato::criteria::extension::test_utilities
