#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SPHEREBUILDER
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SPHEREBUILDER

#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{

using NamedSphereCenterCoord = utilities::NamedType<double, struct NamedSphereCenterCoordTag>;

/// @brief Given num values, @a aNumValues, start, @a aStart, and step, @a aStep, calculate a sequence of
/// coordinates and store them in a vector.
[[nodiscard]] auto calculate_sphere_center_coords(const unsigned int aNumValues,
                                                  NamedSphereCenterCoord aStart,
                                                  const double aStep) -> std::vector<double>;

/// @brief Given sphere pattern input, @a aData, generate sphere primitives to be used to initialize a krino levelset.
[[nodiscard]] auto generate_spheres(const SpherePatternData &aData) -> std::vector<Sphere>;

/// @brief Given the sphere pattern from the user generate sphere center coordinates for all spheres in the pattern.
[[nodiscard]] auto calculate_sphere_locator_data(const SpherePatternData &aData) -> SphereLocatorData;

/// @brief Calculate the number spheres in each direction give sphere pattern data.
[[nodiscard]] auto calculate_num_spheres_in_each_direction(const SpherePatternData &aData) -> common::Coordinate;

/// @brief Calculate the minimum/starting coordinates for the sphere pattern.
[[nodiscard]] auto calculate_sphere_pattern_start(const common::Coordinate &aNumSpheres, const SpherePatternData &aData)
    -> common::Coordinate;

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_SPHERE_BUILDER
