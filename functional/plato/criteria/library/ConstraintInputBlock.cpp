#include "plato/criteria/library/ConstraintInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kConstraintParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::constraint>{};
}
}  // namespace plato::criteria::library
