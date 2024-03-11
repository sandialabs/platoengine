#include "plato/geometry/library/GeometryRegistrationUtilities.hpp"

namespace plato::geometry::library::detail
{
void emplace_back_if_has_value(std::vector<library::GeometryInput>& aGeometryInput,
                               std::optional<library::GeometryInput>&& aOptionalGeometry)
{
    if (aOptionalGeometry.has_value())
    {
        aGeometryInput.emplace_back(std::move(aOptionalGeometry).value());
    }
}

std::vector<library::GeometryInput> geometry_blocks(const input_parser::ParsedInput& aInput)
{
    constexpr auto tNumInputFields = boost::fusion::result_of::size<input_parser::ParsedInput>::value;
    return geometry_blocks_impl(aInput, std::make_index_sequence<tNumInputFields>{});
}

std::optional<library::GeometryInput> first_geometry_block(const input_parser::ParsedInput& aInput)
{
    const std::vector<library::GeometryInput> tGeometryBlocks = geometry_blocks(aInput);
    if (tGeometryBlocks.empty())
    {
        return std::nullopt;
    }
    else
    {
        return tGeometryBlocks.front();
    }
}

std::string block_name(const library::GeometryInput& aInput)
{
    return std::visit(
        [](const auto& aObj) -> std::string
        {
            using InputType = std::decay_t<decltype(aObj)>;
            return input_parser::block_name<InputType>();
        },
        aInput);
}

std::string block_name(const library::ValidatedGeometryInput& aInput)
{
    return std::visit(
        [](const auto& aObj) -> std::string
        {
            using ValidatedInputType = std::decay_t<decltype(aObj)>;
            using RawInputType = typename ValidatedInputType::RawInputType;
            return input_parser::block_name<RawInputType>();
        },
        aInput.rawInput());
}

}  // namespace plato::geometry::library::detail
