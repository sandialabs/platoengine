#ifndef PLATO_UTILITIES_TRANSFORMIF
#define PLATO_UTILITIES_TRANSFORMIF

#include <type_traits>

namespace plato::utilities
{
/// @brief Applies @a aTransformFunction to the elements in @a aContainer that satisfy @a aPredicate.
///  The result is assigned to @a aNewRange, which is then incremented.
///
/// @tparam Container Container type that must be usable in a range-based for loop.
/// @tparam Iterator Must be assignable with the return type of @a TransformFunction and must be incrementable.
/// @tparam TransformFunction Return type must be assignable to @a Iterator
/// @tparam Predicate Return type must be convertible to `bool`.
template <typename Container, typename Iterator, typename TransformFunction, typename Predicate>
constexpr Iterator transform_if(const Container& aContainer,
                                Iterator aNewRange,
                                const TransformFunction& aTransformFunction,
                                const Predicate& aPredicate);

template <typename Container, typename Iterator, typename TransformFunction, typename Predicate>
constexpr Iterator transform_if(const Container& aContainer,
                                Iterator aNewRange,
                                const TransformFunction& aTransformFunction,
                                const Predicate& aPredicate)
{
    using ContainedType = typename Container::value_type;
    static_assert(std::is_invocable_v<Predicate, ContainedType>);
    static_assert(std::is_invocable_v<TransformFunction, ContainedType>);

    using PredicateReturnType = std::invoke_result_t<Predicate, ContainedType>;
    using TransformReturnType = std::invoke_result_t<TransformFunction, ContainedType>;
    using IteratorDereferenceType = decltype(*aNewRange);
    static_assert(std::is_convertible_v<PredicateReturnType, bool>);
    static_assert(std::is_assignable_v<IteratorDereferenceType, TransformReturnType>);

    for (const auto& aValue : aContainer)
    {
        if (aPredicate(aValue))
        {
            *aNewRange = aTransformFunction(aValue);
            ++aNewRange;
        }
    }
    return aNewRange;
}
}  // namespace plato::utilities

#endif
