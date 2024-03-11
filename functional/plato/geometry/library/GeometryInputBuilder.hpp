#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYINPUTBUILDER
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYINPUTBUILDER

#include <boost/fusion/sequence.hpp>
#include <tuple>
#include <variant>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::geometry::library::detail
{
template <typename T>
struct TypeOrOptional
{
    using type = T;
};

template <typename T>
struct TypeOrOptional<boost::optional<T>>
{
    using type = T;
};

template <typename T>
auto tuple_if_geometry_input()
{
    using InputType = typename TypeOrOptional<T>::type;
    if constexpr (input_parser::kIsGeometryInput<InputType>)
    {
        return std::tuple<InputType>{};
    }
    else
    {
        return std::tuple<>{};
    }
}

template <typename FusionStruct, std::size_t Index, typename TupleToCat>
auto cat_if_is_geometry_input(TupleToCat&& tTupleToCat)
{
    auto tNewTuple = std::tuple_cat(
        std::forward<TupleToCat>(tTupleToCat),
        tuple_if_geometry_input<typename boost::fusion::result_of::value_at_c<FusionStruct, Index>::type>());
    if constexpr (Index == 0)
    {
        return tNewTuple;
    }
    else
    {
        return cat_if_is_geometry_input<FusionStruct, Index - 1>(std::move(tNewTuple));
    }
}

struct make_geometry_input_tuple
{
    template <typename FusionStruct>
    auto operator()(FusionStruct)
    {
        constexpr auto tStructSize = boost::fusion::result_of::size<FusionStruct>::value;
        static_assert(tStructSize > 0, "Fusion struct must not be empty");
        return cat_if_is_geometry_input<FusionStruct, tStructSize - 1>(std::tuple<>{});
    }
};

template <typename Tuple>
struct VariantFromTuple;

template <typename... Ts>
struct VariantFromTuple<std::tuple<Ts...>>
{
    using type = std::variant<Ts...>;
    using validated_type = std::variant<core::ValidatedInputTypeWrapper<Ts>...>;
};

template <typename FusionStruct>
using GeometryInputVariant =
    typename VariantFromTuple<std::invoke_result_t<make_geometry_input_tuple, FusionStruct>>::type;

template <typename FusionStruct>
using ValidatedGeometryInputVariant =
    typename VariantFromTuple<std::invoke_result_t<make_geometry_input_tuple, FusionStruct>>::validated_type;

}  // namespace plato::geometry::library::detail

#endif
