#include "plato/criteria/library/ObjectiveInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/EnumTypeHelpers.hpp"

DEFINE_ENUM_SYMBOL_TABLE(ObjectiveGoal, plato::criteria::library)

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] const auto kObjectiveParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::objective>{};
}
}  // namespace plato::criteria::library
