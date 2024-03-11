#ifndef PLATO_UTILITIES_PARAMETERBOUNDS
#define PLATO_UTILITIES_PARAMETERBOUNDS

#include <functional>
#include <string>
#include <type_traits>

#include "plato/utilities/BoundClusivity.hpp"

namespace plato::utilities
{
/// @brief A helper type for defining the bounds of an input parameter.
///
/// Bounds may be inclusive, exclusive, or unbounded. For example, bounds on
/// [0, 1] on real numbers may be expressed as
/// @code{.cpp}
/// ParameterBounds(Inclusive{0.0}, Inclusive{1.0})
/// @endcode
///
/// @tparam A numeric type.
/// @sa lower_bounded
/// @sa upper_bounded
template <typename T>
class ParameterBounds
{
   public:
    template <typename Lower, typename Upper>
    ParameterBounds(Lower aLower, Upper aUpper);

    [[nodiscard]] bool contains(const T& aValue) const;
    [[nodiscard]] std::string description() const;

   private:
    T mLowerBound{};
    detail::Clusivity mLowerClusivity;
    T mUpperBound{};
    detail::Clusivity mUpperClusivity;
};

/// @brief Helper for constructing a ParameterBounds object on [0, 1].
[[nodiscard]] auto unit_bounded() -> ParameterBounds<double>;

/// @brief Helper for constructing a ParameterBounds object with a defined lower bound
///  and is unbounded above.
template <typename Bound>
[[nodiscard]] auto lower_bounded(const Bound aLowerBound) -> ParameterBounds<typename Bound::type>;

/// @brief Helper for constructing a ParameterBounds object with a defined upper bound
///  and is unbounded below.
template <typename Bound>
[[nodiscard]] auto upper_bounded(const Bound aUpperBound) -> ParameterBounds<typename Bound::type>;

/// @brief Helper for constructing a ParameterBounds object with no bounds, so
///  calling `contains` is always `true`.
template <typename T>
[[nodiscard]] auto unbounded() -> ParameterBounds<T>;

// CTAD deduction guides
template <typename Lower, typename Upper>
ParameterBounds(Lower, Upper) -> ParameterBounds<typename Lower::type>;

template <typename T>
template <typename Lower, typename Upper>
ParameterBounds<T>::ParameterBounds(Lower aLower, Upper aUpper)
    : mLowerBound{aLower.mValue},
      mLowerClusivity{Lower::mClusivity},
      mUpperBound{aUpper.mValue},
      mUpperClusivity{Upper::mClusivity}
{
    static_assert(std::is_same_v<typename Lower::type, typename Upper::type>,
                  "ParameterBounds must be constructed with the same types for both bounds.");

    static_assert(std::is_arithmetic_v<typename Lower::type>,
                  "ParameterBounds must be instantiated with an arithmetic type.");
}

namespace detail
{
template <typename T, typename Comparison>
bool bound_satisfied(const T aValue, const detail::Clusivity aClusivity, const T aBound, const Comparison& aComparison)
{
    const bool tUnbounded = aClusivity == detail::Clusivity::kUnbounded;
    const bool tExclusivelySatisfied = aComparison(aValue, aBound);
    const bool tAtBound = aValue == aBound;
    const bool tBoundSatisfied =
        aClusivity == detail::Clusivity::kInclusive ? tAtBound || tExclusivelySatisfied : tExclusivelySatisfied;
    return tUnbounded || tBoundSatisfied;
}

template <typename T>
bool lower_bound_satisfied(const T aValue, const detail::Clusivity aClusivity, const T aBound)
{
    return bound_satisfied(aValue, aClusivity, aBound, std::greater{});
}

template <typename T>
bool upper_bound_satisfied(const T aValue, const detail::Clusivity aClusivity, const T aBound)
{
    return bound_satisfied(aValue, aClusivity, aBound, std::less{});
}
}  // namespace detail

template <typename T>
bool ParameterBounds<T>::contains(const T& aValue) const
{
    const bool tLowerSatisfied = detail::lower_bound_satisfied(aValue, mLowerClusivity, mLowerBound);
    const bool tUpperSatisfied = detail::upper_bound_satisfied(aValue, mUpperClusivity, mUpperBound);
    return tLowerSatisfied && tUpperSatisfied;
}

namespace detail
{
template <typename T>
std::string lower_description(const T aValue, const Clusivity aClusivity)
{
    if (aClusivity == Clusivity::kInclusive)
    {
        return "[" + std::to_string(aValue);
    }
    else if (aClusivity == Clusivity::kExclusive)
    {
        return "(" + std::to_string(aValue);
    }
    else
    {
        return "(-inf";
    }
}

template <typename T>
std::string upper_description(const T aValue, const Clusivity aClusivity)
{
    if (aClusivity == Clusivity::kInclusive)
    {
        return std::to_string(aValue) + "]";
    }
    else if (aClusivity == Clusivity::kExclusive)
    {
        return std::to_string(aValue) + ")";
    }
    else
    {
        return "inf)";
    }
}

}  // namespace detail

template <typename T>
std::string ParameterBounds<T>::description() const
{
    return lower_description(mLowerBound, mLowerClusivity) + ", " + upper_description(mUpperBound, mUpperClusivity);
}

template <typename Bound>
[[nodiscard]] auto lower_bounded(const Bound aLowerBound) -> ParameterBounds<typename Bound::type>
{
    return ParameterBounds{aLowerBound, Unbounded<typename Bound::type>{}};
}

template <typename Bound>
[[nodiscard]] auto upper_bounded(const Bound aUpperBound) -> ParameterBounds<typename Bound::type>
{
    return ParameterBounds{Unbounded<typename Bound::type>{}, aUpperBound};
}

template <typename T>
[[nodiscard]] auto unbounded() -> ParameterBounds<T>
{
    return ParameterBounds{Unbounded<T>{}, Unbounded<T>{}};
}

}  // namespace plato::utilities

#endif
