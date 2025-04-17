#include "plato/geometry/library/GeometryValidation.hpp"

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"

namespace plato::geometry::library
{
[[maybe_unused]] static auto kGeometryValidationRegistration = input_validation::NewParsedInputValidationRegistration<>{
    [](const input_parser::NewParsedInput& aInput) { return detail::validate_only_one_geometry(aInput); }};

namespace detail
{
auto validate_only_one_geometry(const input_parser::NewParsedInput& aInput) -> std::optional<std::string>
{
    if (const auto tTally = aInput.get<input_parser::ComponentType::kGeometry>().size(); tTally != 1U)
    {
        return "Only define exactly one geometry block. There were " + std::to_string(tTally) + " found.";
    }
    else
    {
        return std::nullopt;
    }
}

}  // namespace detail
}  // namespace plato::geometry::library
