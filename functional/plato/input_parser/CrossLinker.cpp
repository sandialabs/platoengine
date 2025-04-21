#include "plato/input_parser/CrossLinker.hpp"

namespace plato::input_parser
{
auto CrossLinker::crossLink(InputDataBlock aInputBlock, const ParsedInput& aParsedInput) const
    -> CrossLinkedBlockOrError
{
    return mCrossLinkFunction(std::move(aInputBlock), aParsedInput);
}
}  // namespace plato::input_parser
