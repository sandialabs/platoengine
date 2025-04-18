#include "plato/criteria/library/ConstraintInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/InputDefinitions.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kConstraintParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::new_constraint>{};
}

auto create_valid_example_constraint_input() -> input_parser::new_constraint
{
    return input_parser::new_constraint{
        /*.name=*/std::string{"bike-shed"},
        /*.active=*/true,
        /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
        /*.criterion=*/input_parser::CriterionName{"nodal_sum"},
        /*.number_of_processors=*/1U,
        /*.input_files=*/input_parser::FileList{{"brown.txt", "butter.txt", "sauce.txt"}},
        /*.constraint_value=*/0.0,
        /*.is_linear=*/true,
        /*.constraint_type=*/input_parser::ConstraintTypes::kEqualTo};
}
}  // namespace plato::criteria::library
