#include "plato/geometry/library/GeometryRegistration.hpp"

#include "plato/geometry/library/GeometryRegistrationUtilities.hpp"

namespace plato::geometry::library
{
bool is_geometry_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<FactoryTypes, ValidatedGeometryInput>(aFunctionName);
}

library::GeometryInput first_geometry_input(const input_parser::ParsedInput& aInput)
{
    const std::optional<library::GeometryInput> tGeometryInput = detail::first_geometry_block(aInput);
    if (!tGeometryInput)
    {
        throw plato::utilities::Exception("No geometry block was defined.");
    }
    return tGeometryInput.value();
}

}  // namespace plato::geometry::library