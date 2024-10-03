#ifndef PLATO_UTILITIES_BOOSTOPTIONALTOOPTIONAL
#define PLATO_UTILITIES_BOOSTOPTIONALTOOPTIONAL

#include <boost/optional.hpp>
#include <optional>

namespace plato::utilities
{

template <typename BoostOptionalT>
[[nodiscard]] auto to_std_optional(BoostOptionalT&& aOptional)
    -> std::optional<typename std::decay_t<BoostOptionalT>::value_type>
{
    if (aOptional)
    {
        using ValueType = typename std::decay_t<BoostOptionalT>::value_type;
        return std::optional<ValueType>{std::forward<BoostOptionalT>(aOptional).value()};
    }
    return std::nullopt;
}

template <typename BoostOptionalT, typename F>
[[nodiscard]] auto to_unwrapped_optional(BoostOptionalT&& aOptional, const F& aUnwrapFunction)
    -> std::optional<std::invoke_result_t<F, typename std::decay_t<BoostOptionalT>::value_type>>
{
    if (aOptional)
    {
        return std::make_optional(aUnwrapFunction(std::forward<BoostOptionalT>(aOptional).value()));
    }
    return std::nullopt;
}

}  // namespace plato::utilities

#endif
