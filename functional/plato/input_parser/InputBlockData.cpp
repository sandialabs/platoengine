#include "plato/input_parser/InputBlockData.hpp"

namespace plato::input_parser
{
auto InputBlockWrapper::hasValue() const -> bool { return mInput.has_value(); }

auto InputBlockWrapper::active() const -> bool { return mActive(mInput); }

}  // namespace plato::input_parser
