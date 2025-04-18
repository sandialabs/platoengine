#include "plato/criteria/library/ObjectiveInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputDefinitions.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kObjectiveParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::objective>{};
}

[[nodiscard]] auto create_valid_example_objective_input() -> input_parser::objective
{
    return input_parser::objective{/*.name=*/std::string{"bike-shed"},
                                   /*.active=*/true,
                                   /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                                   /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
                                   /*.number_of_processors=*/1u,
                                   /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
                                   /*.aggregation_weight=*/13.0};
}
}  // namespace plato::criteria::library
