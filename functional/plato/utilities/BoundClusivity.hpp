#ifndef PLATO_UTILITIES_BOUNDCLUSIVITY
#define PLATO_UTILITIES_BOUNDCLUSIVITY

#include <type_traits>

namespace plato::utilities
{
namespace detail
{
enum struct Clusivity
{
    kInclusive,
    kExclusive,
    kUnbounded
};
}  // namespace detail

/// @brief Helper type for constructing ParameterBounds.
///
/// This defines an exclusive bound, meaning that values equal to this
/// bound are considered not in the bounds.
/// @note This and Inclusive and Unbounded can be consolidated into a template
///  and defined with aliases in C++20. They're currently defined separately to
///  allow CTAD.
template <typename T>
struct Exclusive
{
    using type = T;
    T mValue{};
    static constexpr detail::Clusivity mClusivity = detail::Clusivity::kExclusive;
};

/// @brief Helper type for constructing ParameterBounds.
///
/// This defines an inclusive bound, meaning that values equal to this
/// bound are considered in the bounds.
template <typename T>
struct Inclusive
{
    using type = T;
    T mValue{};
    static constexpr detail::Clusivity mClusivity = detail::Clusivity::kInclusive;
};

/// @brief Helper type for constructing ParameterBounds.
///
/// This is unbounded, meaning that any values satisfy this bound.
template <typename T>
struct Unbounded
{
    using type = T;
    T mValue{};
    static constexpr detail::Clusivity mClusivity = detail::Clusivity::kUnbounded;
};

// Deduction guides
template <typename T>
Inclusive(T) -> Inclusive<T>;

template <typename T>
Exclusive(T) -> Exclusive<T>;
}  // namespace plato::utilities

#endif
