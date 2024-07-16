#ifndef PLATO_CORE_LIBRARY_VARIANTINPUTBUILDER
#define PLATO_CORE_LIBRARY_VARIANTINPUTBUILDER

#include <boost/fusion/sequence.hpp>
#include <boost/optional.hpp>
#include <tuple>
#include <variant>

#include "plato/core/ValidatedInputTypeWrapper.hpp"

namespace plato::core
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
struct TypeOrValidatedType
{
    using type = T;
};

template <typename T>
struct TypeOrValidatedType<ValidatedInputTypeWrapper<T>>
{
    using type = T;
};

template <typename T, template <typename> typename IsVariantMember>
auto tuple_if_variant_input()
{
    using InputType = typename TypeOrOptional<T>::type;
    if constexpr (IsVariantMember<InputType>::value)
    {
        return std::tuple<InputType>{};
    }
    else
    {
        return std::tuple<>{};
    }
}

template <typename FusionStruct, template <typename> typename IsVariantMember, std::size_t Index, typename TupleToCat>
auto cat_if_is_variant_input(TupleToCat&& tTupleToCat)
{
    auto tNewTuple =
        std::tuple_cat(std::forward<TupleToCat>(tTupleToCat),
                       tuple_if_variant_input<typename boost::fusion::result_of::value_at_c<FusionStruct, Index>::type,
                                              IsVariantMember>());
    if constexpr (Index == 0)
    {
        return tNewTuple;
    }
    else
    {
        return cat_if_is_variant_input<FusionStruct, IsVariantMember, Index - 1>(std::move(tNewTuple));
    }
}

template <template <typename> typename IsVariantMember>
struct make_variant_input_tuple
{
    template <typename FusionStruct>
    auto operator()(FusionStruct)
    {
        constexpr auto tStructSize = boost::fusion::result_of::size<FusionStruct>::value;
        static_assert(tStructSize > 0, "Fusion struct must not be empty");
        return cat_if_is_variant_input<FusionStruct, IsVariantMember, tStructSize - 1>(std::tuple<>{});
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

template <typename FusionStruct, template <typename> typename IsVariantMember>
using InputVariant =
    typename VariantFromTuple<std::invoke_result_t<make_variant_input_tuple<IsVariantMember>, FusionStruct>>::type;

template <typename FusionStruct, template <typename> typename IsVariantMember>
using ValidatedInputVariant = typename VariantFromTuple<
    std::invoke_result_t<make_variant_input_tuple<IsVariantMember>, FusionStruct>>::validated_type;

}  // namespace plato::core

#endif
