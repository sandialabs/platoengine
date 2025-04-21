#include "plato/input_parser/CrossReference.hpp"

namespace plato::input_parser
{
auto InputBlockWrapper::hasValue() const -> bool { return mInput.has_value(); }

}  // namespace plato::input_parser
