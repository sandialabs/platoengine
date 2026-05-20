#include "plato/geometry/library/GeometryFactory.hpp"

#include <utility>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::library
{
auto make_geometry_data(const ValidatedGeometryInput& aGeometryInput) -> FactoryTypes
{
    if (auto tGeometry = core::create_object_from_factory<FactoryTypes, ValidatedGeometryInput>(
            aGeometryInput.rawInput().mBlockName, aGeometryInput))
    {
        return std::move(tGeometry).value();
    }

    throw utilities::Exception{"Unknown geometry"};
}

}  // namespace plato::geometry::library
