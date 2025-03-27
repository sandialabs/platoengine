#include "plato/input_parser/ComponentBlockParser.hpp"

namespace plato::input_parser
{
auto ComponentBlockParser::parse(const GenericBlockData& aData) const -> InputDataBlock
{
    return mParseFunction(aData);
}

}  // namespace plato::input_parser
