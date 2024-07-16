#ifndef PLATO_PROCESSMANAGER_LIBRARY_CROSSREFERENCEUTILITIES
#define PLATO_PROCESSMANAGER_LIBRARY_CROSSREFERENCEUTILITIES

#include <boost/optional.hpp>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/CrossReference.hpp"

namespace plato::process_manager::library
{
/// @brief Given a boost fusion struct @a aInput containing input block structs, applies @a aApplyFunction to
/// each CrossReference type found in @a aInput.
template <typename FullInput, typename ApplyFunction>
void apply_to_cross_references(FullInput& aInput, const ApplyFunction& aApplyFunction);

namespace detail
{
/// @brief A type trait specifying if a type is a CrossReference templated on @a T
template <typename T>
constexpr bool kIsCrossReference = false;

template <template <typename> typename TypeTrait>
constexpr bool kIsCrossReference<plato::input_parser::CrossReference<TypeTrait>> = true;

/// @brief A function object that applies a function to all elements of a container or
///  the type contained in an optional.
///
/// The main purpose of this is provide a uniform interface for iterating over containers and optionals.
template <template <typename...> typename Container, typename... T>
struct apply_to_elements
{
    Container<T...>& mContainer;

    template <typename F>
    void operator()(const F& f)
    {
        for (auto& tElement : mContainer)
        {
            f(tElement);
        }
    }
};

template <template <typename...> typename Container, typename... T>
apply_to_elements(Container<T...>&) -> apply_to_elements<Container, T...>;

template <typename T>
struct apply_to_elements<boost::optional, T>
{
    boost::optional<T>& mOptional;

    template <typename F>
    void operator()(const F& f)
    {
        if (mOptional)
        {
            f(mOptional.value());
        }
    }
};

template <typename InputBlock, typename FullInput, typename ApplyFunction>
void apply_to_input_block_cross_reference(InputBlock& aInputBlock,
                                          const FullInput& aFullInput,
                                          const ApplyFunction& aApplyFunction)
{
    boost::fusion::for_each(aInputBlock,
                            [&aInputBlock, &aFullInput, &aApplyFunction](auto& aField)
                            {
                                using FieldType =
                                    typename plato::core::TypeOrOptional<std::decay_t<decltype(aField)>>::type;
                                if constexpr (detail::kIsCrossReference<FieldType>)
                                {
                                    aApplyFunction(aField, aInputBlock, aFullInput);
                                }
                            });
}

template <typename CrossReferenceType, typename VariantType>
[[nodiscard]] auto find_cross_reference_named_block(const CrossReferenceType& aField,
                                                    const std::vector<VariantType>& aAllLinkableBlocks)
{
    const auto tCrossReferenceName = aField.mName;
    return std::find_if(aAllLinkableBlocks.cbegin(), aAllLinkableBlocks.cend(),
                        [&tCrossReferenceName](const auto& aVariant)
                        { return core::block_name(aVariant) == tCrossReferenceName; });
}

}  // namespace detail

template <typename FullInput, typename ApplyFunction>
void apply_to_cross_references(FullInput& aInput, const ApplyFunction& aApplyFunction)
{
    boost::fusion::for_each(aInput,
                            [&aInput, &aApplyFunction](auto& aInputBlock)
                            {
                                detail::apply_to_elements{aInputBlock}(
                                    [&aInput, &aApplyFunction](auto& aBlock)
                                    { detail::apply_to_input_block_cross_reference(aBlock, aInput, aApplyFunction); });
                            });
}

}  // namespace plato::process_manager::library

#endif