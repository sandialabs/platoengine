#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATIONUTILITIES
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATIONUTILITIES

#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::library::detail
{
/// @brief A type trait specifying if a type @a T is an alternative of variant @a VariantT
template <typename T, typename VariantT>
constexpr bool kIsVariantMember = false;

template <typename T, typename... Ts>
constexpr bool kIsVariantMember<T, std::variant<Ts...>> = std::disjunction_v<std::is_same<T, Ts>...>;

template <typename T>
[[nodiscard]] std::optional<library::GeometryInput> make_variant_if_geometry(const T&)
{
    return std::nullopt;
}

/// @return If @a aT is an alternative of variant GeometryInput, and is non-empty, returns an optional-wrapped copy of
/// @a aT.
template <typename T>
[[nodiscard]] std::optional<library::GeometryInput> make_variant_if_geometry(const boost::optional<T>& aT)
{
    if (kIsVariantMember<T, library::GeometryInput> && aT)
    {
        return std::make_optional(library::GeometryInput{aT.value()});
    }
    return std::nullopt;
}

void emplace_back_if_has_value(std::vector<library::GeometryInput>& aGeometryInput,
                               std::optional<library::GeometryInput>&& aOptionalGeometry);

template <std::size_t... Is>
[[nodiscard]] std::vector<library::GeometryInput> geometry_blocks_impl(const input_parser::ParsedInput& aInput,
                                                                       std::integer_sequence<std::size_t, Is...>)
{
    std::vector<library::GeometryInput> tGeometryInput;
    (emplace_back_if_has_value(tGeometryInput, make_variant_if_geometry(boost::fusion::at_c<Is>(aInput))), ...);
    return tGeometryInput;
}
[[nodiscard]] std::vector<library::GeometryInput> geometry_blocks(const input_parser::ParsedInput& aInput);

/// @return An optional GeometryInput variant, which is the first non-empty geometry input block found in @a aInput.
///  If no geometry block was found, an empty optional is returned.
[[nodiscard]] std::optional<library::GeometryInput> first_geometry_block(const input_parser::ParsedInput& aInput);

/// @return The name of the geometry input held by the variant @a aInput
[[nodiscard]] std::string block_name(const library::GeometryInput& aInput);

/// @return The name of the geometry input held by the variant @a aInput
[[nodiscard]] std::string block_name(const library::ValidatedGeometryInput& aInput);

}  // namespace plato::geometry::library::detail

#endif
