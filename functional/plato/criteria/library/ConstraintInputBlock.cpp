#include "plato/criteria/library/ConstraintInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"

DEFINE_ENUM_SYMBOL_TABLE(ConstraintTypes, plato::input_parser)

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kConstraintParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::constraint>{};
}
}  // namespace plato::criteria::library
