#include <gtest/gtest.h>

#include "plato/utilities/TupleHelpers.hpp"
#include "plato/utilities/unittest/CopyCounter.hpp"

namespace plato::utilities::unittest
{
namespace
{
template <std::size_t kIndex, std::size_t... kIndices>
constexpr auto index_from_index_sequence(std::index_sequence<kIndices...>)
{
    constexpr auto tArray = std::array{kIndices...};
    return tArray[kIndex];
}
}  // namespace

TEST(TupleHelpers, MakeIndexSequenceStartingFrom)
{
    {
        constexpr auto tTestSequence = detail::make_index_sequence_starting_from<0>(std::make_index_sequence<2>());
        static_assert(tTestSequence.size() == 2);
        static_assert(index_from_index_sequence<0>(tTestSequence) == 0);
        static_assert(index_from_index_sequence<1>(tTestSequence) == 1);
    }
    {
        constexpr auto tTestSequence = detail::make_index_sequence_starting_from<5>(std::make_index_sequence<2>());
        static_assert(tTestSequence.size() == 2);
        static_assert(index_from_index_sequence<0>(tTestSequence) == 5);
        static_assert(index_from_index_sequence<1>(tTestSequence) == 6);
    }
}

TEST(TupleHelpers, HasType)
{
    using Tuple1 = std::tuple<double>;
    static_assert(detail::has_type<double, Tuple1>(std::make_index_sequence<std::tuple_size_v<Tuple1>>()));
    static_assert(!detail::has_type<char, Tuple1>(std::make_index_sequence<std::tuple_size_v<Tuple1>>()));

    using Tuple2 = std::tuple<double, int>;
    static_assert(detail::has_type<double, Tuple2>(std::make_index_sequence<std::tuple_size_v<Tuple2>>()));
    static_assert(detail::has_type<int, Tuple2>(std::make_index_sequence<std::tuple_size_v<Tuple2>>()));
    static_assert(!detail::has_type<char, Tuple2>(std::make_index_sequence<std::tuple_size_v<Tuple2>>()));

    // Empty tuple
    static_assert(!detail::has_type<char, std::tuple<>>(std::make_index_sequence<std::tuple_size_v<std::tuple<>>>()));
}

TEST(TupleHelpers, HasUniqueTypes)
{
    // Valid
    using Tuple1 = std::tuple<int>;
    using Tuple2 = std::tuple<int, double>;

    static_assert(detail::has_unique_types<Tuple1>());
    static_assert(detail::has_unique_types<Tuple2>());

    // Invalid
    using Tuple3 = std::tuple<int, int>;
    using Tuple4 = std::tuple<int, double, int>;
    static_assert(!detail::has_unique_types<Tuple3>());
    static_assert(!detail::has_unique_types<Tuple4>());
}

TEST(TupleHelpers, TupleIsSubset)
{
    // Valid
    using Tuple0 = std::tuple<>;
    using Tuple1 = std::tuple<int>;
    using Tuple2 = std::tuple<int, double>;

    static_assert(detail::tuple_is_subset<Tuple0, Tuple1>());
    static_assert(detail::tuple_is_subset<Tuple0, Tuple2>());
    static_assert(detail::tuple_is_subset<Tuple1, Tuple2>());
    static_assert(detail::tuple_is_subset<Tuple0, Tuple0>());
    static_assert(detail::tuple_is_subset<Tuple2, Tuple2>());

    // Invalid
    static_assert(!detail::tuple_is_subset<Tuple2, Tuple0>());
    static_assert(!detail::tuple_is_subset<Tuple2, Tuple1>());
    static_assert(!detail::tuple_is_subset<Tuple1, Tuple0>());
}

TEST(TupleHelpers, AreTuplesCompatibleDegenerateCases)
{
    using Tuple1 = std::tuple<>;
    using Tuple2 = std::tuple<int, double, bool>;

    static_assert(are_tuples_compatible<Tuple1, Tuple2>());
    static_assert(!are_tuples_compatible<Tuple2, Tuple1>());
}

TEST(TupleHelpers, AreTuplesCompatible)
{
    using Tuple1 = std::tuple<int, double, bool>;
    using Tuple2 = std::tuple<int, double, bool>;
    using Tuple3 = std::tuple<int, double>;
    using Tuple4 = std::tuple<int>;
    using Tuple5 = std::tuple<double, int>;
    using Tuple6 = std::tuple<std::vector<double>>;

    // Copyable
    static_assert(are_tuples_compatible<Tuple1, Tuple2>());
    static_assert(are_tuples_compatible<Tuple3, Tuple1>());
    static_assert(are_tuples_compatible<Tuple4, Tuple1>());
    static_assert(are_tuples_compatible<Tuple5, Tuple1>());
    // Not copyable
    static_assert(!are_tuples_compatible<Tuple1, Tuple3>());
    static_assert(!are_tuples_compatible<Tuple1, Tuple4>());
    static_assert(!are_tuples_compatible<Tuple1, Tuple5>());

    static_assert(!are_tuples_compatible<Tuple1, Tuple6>());
    static_assert(!are_tuples_compatible<Tuple2, Tuple6>());
    static_assert(!are_tuples_compatible<Tuple3, Tuple6>());
    static_assert(!are_tuples_compatible<Tuple4, Tuple6>());
    static_assert(!are_tuples_compatible<Tuple5, Tuple6>());

    // Invalid due to non-unique types
    using TupleTwoDoubles = std::tuple<double, double>;
    using TupleOneDouble = std::tuple<double>;

    static_assert(!are_tuples_compatible<TupleOneDouble, TupleTwoDoubles>());
    static_assert(!are_tuples_compatible<TupleTwoDoubles, TupleOneDouble>());
}

}  // namespace plato::utilities::unittest
