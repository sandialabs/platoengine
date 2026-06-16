#include "plato/input_parser/ComponentBlockParser.hpp"

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"

namespace plato::input_parser
{
auto ComponentBlockParser::parse(const GenericBlockData& aData) const -> ParsedDataOrError
{
    return mParseFunction(aData);
}

auto ComponentBlockParser::componentType() const -> components::ComponentType { return mComponentType; }

}  // namespace plato::input_parser
