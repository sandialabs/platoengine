#include "plato/input_parser/CrossLinker.hpp"

#include <utility>

#include "plato/input_parser/InputBlockData.hpp"
#include "plato/input_parser/ParsedInput.hpp"

namespace plato::input_parser
{
auto CrossLinker::crossLink(InputDataBlock aInputBlock, const ParsedInput& aParsedInput) const
    -> CrossLinkedBlockOrError
{
    return mCrossLinkFunction(std::move(aInputBlock), aParsedInput);
}
}  // namespace plato::input_parser
