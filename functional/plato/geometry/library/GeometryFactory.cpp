#include "plato/geometry/library/GeometryFactory.hpp"

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::library
{
auto make_geometry_data(const NewValidatedGeometryInput& aGeometryInput) -> FactoryTypes
{
    if (auto tGeometry = core::create_object_from_factory<FactoryTypes, NewValidatedGeometryInput>(
            aGeometryInput.rawInput().mBlockName, aGeometryInput))
    {
        return std::move(tGeometry).value();
    }
    else
    {
        throw utilities::Exception{"Unknown geometry"};
    }
}

}  // namespace plato::geometry::library
