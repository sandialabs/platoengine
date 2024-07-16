#include <optional>
#include <vector>

namespace plato::utilities
{
/// @brief Converts an optional to a vector, with an empty optional resulting in an empty vector
template <typename OptionalT>
[[nodiscard]] auto optional_to_vector(OptionalT&& aOptional);

namespace detail
{
template <typename T>
constexpr bool kIsOptional = false;

template <typename T>
constexpr bool kIsOptional<std::optional<T>> = true;
}  // namespace detail

template <typename OptionalT>
auto optional_to_vector(OptionalT&& aOptional)
{
    static_assert(detail::kIsOptional<std::decay_t<OptionalT>>,
                  "optional_to_vector must be called with a std::optional");
    using T = typename std::decay_t<OptionalT>::value_type;

    if (aOptional.has_value())
    {
        auto tVector = std::vector<T>{};
        tVector.emplace_back(std::forward<OptionalT>(aOptional).value());  // NOLINT
        return tVector;
    }
    else
    {
        return std::vector<T>{};
    }
}
}  // namespace plato::utilities
