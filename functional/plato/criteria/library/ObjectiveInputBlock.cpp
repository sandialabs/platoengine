#include "plato/criteria/library/ObjectiveInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kObjectiveParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::objective>{};
}
}  // namespace plato::criteria::library
