#include "plato/input_parser/CrossLinker.hpp"

namespace plato::input_parser
{
void CrossLinker::crossLink(InputDataBlock& aInputBlock, const NewParsedInput& aNewParsedInput) const
{
    mCrossLinkFunction(aInputBlock, aNewParsedInput);
}
}  // namespace plato::input_parser
