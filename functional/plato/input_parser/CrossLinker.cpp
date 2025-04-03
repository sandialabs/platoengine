#include "plato/input_parser/CrossLinker.hpp"

namespace plato::input_parser
{
auto CrossLinker::crossLink(InputDataBlock aInputBlock, const NewParsedInput& aNewParsedInput) const
    -> CrossLinkedBlockOrError
{
    return mCrossLinkFunction(std::move(aInputBlock), aNewParsedInput);
}
}  // namespace plato::input_parser
