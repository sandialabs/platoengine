#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYFACTORY
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYFACTORY

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace input_parser
{
struct ParsedInput;
}

namespace plato::geometry::library
{
using DesignParameters = linear_algebra::DynamicVector<double>;
using GeometryFunction =
    core::Function<core::MeshProxy, linear_algebra::JacobianMultiplier, const linear_algebra::DynamicVector<double>&>;

/// @brief Factory function for creating all geometry data, including a GeometryFunction, an initial guess,
///  the bound constraints, and an output function.
///
/// A GeometryFunction is the main geometry representation. It transforms design variables to
/// a mesh or density field.
/// The initial guess is generated from the specific geometry representation, such as
/// a uniform density field.
/// The bound constraints are generated from the specific geometry representation, such as
/// all 0 lower bounds and all 1 upper bounds for density topology optimization.
[[nodiscard]] FactoryTypes make_geometry_data(const ValidatedGeometryInput& aGeometryInput);

}  // namespace plato::geometry::library

#endif
