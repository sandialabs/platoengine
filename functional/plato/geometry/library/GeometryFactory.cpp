#include "plato/geometry/library/GeometryFactory.hpp"

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::library
{
FactoryTypes make_geometry_data(const ValidatedGeometryInput& aGeometryInput)
{
    std::optional<FactoryTypes> tGeometry = core::create_object_from_factory<FactoryTypes, ValidatedGeometryInput>(
        core::block_name(aGeometryInput.rawInput()), aGeometryInput);
    if (tGeometry)
    {
        return std::move(tGeometry).value();
    }
    else
    {
        throw utilities::Exception{"Unknown geometry"};
    }
}
}  // namespace plato::geometry::library
