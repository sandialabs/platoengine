#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_WEDGE
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_WEDGE

#include "plato/filter/extension/kernel_filters/Plane.hpp"

namespace plato::filter::extension::kernel_filters
{

/// @brief Defines a wedge that is made of two planes and a number of repeats to define a mirrored description of a
/// larger domain
/// @pre mRepeats is nonzero.
struct Wedge
{
    Plane mFirstPlane = Plane({0, 0, 0}, {1, 0, 0});
    Plane mSecondPlane = Plane({0, 0, 0}, {0, 1, 0});

    unsigned int mRepeats = 3;
};

/// @brief Using @a aWedgeAngle in degrees, create a Wedge object in the positive quadrant using the xplane and a second
/// plane at the wedge angle. The number of repeats is set to fill the 360 degree space
///@pre @a aWedgeAngle must be commensurate with 360 degrees
[[nodiscard]] auto positive_quadrant_wedge(const double aWedgeAngle) -> Wedge;

/// @brief Take a wedge description in @a aWedge and convert it to a vector of planes that points can be reflected in.
/// @pre mRepeats in aWedge must not be zero
[[nodiscard]] auto wedge_to_planes(const Wedge& aWedge) -> std::vector<Plane>;

}  // namespace plato::filter::extension::kernel_filters

#endif
