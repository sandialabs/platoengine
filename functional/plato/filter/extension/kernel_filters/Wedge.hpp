#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_WEDGE
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_WEDGE

#include "plato/filter/extension/kernel_filters/Plane.hpp"

namespace plato::filter::extension::kernel_filters
{

/// @brief Defines a wedge that is made of two planes and a number of repeats to define a mirrored description of a
/// larger domain
struct Wedge
{
    Plane mFirstPlane = Plane({0, 0, 0}, {1, 0, 0});
    Plane mSecondPlane = Plane({0, 0, 0}, {0, 1, 0});

    unsigned int mRepeats = 3;
};

///@brief Using @a aWedgeAngle in degrees, create a Wedge object in the positive quadrant using the xplane and a second
/// plane at the wedge angle. The number of repeats is set to fill the 360 degree space
///@pre @a aWedgeAngle must be commensurate with 360 degrees
[[nodiscard]] auto positive_quadrant_wedge(const double aWedgeAngle) -> Wedge;

}  // namespace plato::filter::extension::kernel_filters

#endif
