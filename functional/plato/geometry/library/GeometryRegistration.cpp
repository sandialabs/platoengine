#include "plato/geometry/library/GeometryRegistration.hpp"

namespace plato::geometry::library
{
auto is_geometry_function_registered(const std::string_view aFunctionName) -> bool
{
    return core::is_factory_function_registered<FactoryTypes, ValidatedGeometryInput>(aFunctionName);
}

}  // namespace plato::geometry::library
