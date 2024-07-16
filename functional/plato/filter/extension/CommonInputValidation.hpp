#ifndef PLATO_FILTER_EXTENSION_COMMONINPUTVALIDATION
#define PLATO_FILTER_EXTENSION_COMMONINPUTVALIDATION

#include <optional>
#include <string>

#include "plato/core/ValidationUtilities.hpp"

namespace plato::filter::extension::detail
{
template <typename Filter>
std::optional<std::string> validate_filter_radius(const Filter& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<Filter>(), aInput.filter_radius,
                                                           "filter_radius", pfu::lower_bounded(pfu::Exclusive{0.0}));
}
}  // namespace plato::filter::extension::detail

#endif