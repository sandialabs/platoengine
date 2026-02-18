#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_PLANE
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_PLANE

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"

namespace plato::filter::extension::kernel_filters
{

/// @brief Defines a plane given a center and unit normal and allows for points to be relfected in the plane.
struct Plane
{
    third_party_integration::common::Coordinate mCenter = {0, 0, 0};
    third_party_integration::common::UnitVector3 mUnitNormal = {0, 1, 0};

    ///@brief Take a coordinate @a aCoordinate and return a point reflected in the plane
    [[nodiscard]] auto reflect(const third_party_integration::common::Coordinate& aCoordinate) const
        -> third_party_integration::common::Coordinate;

    ///@brief Take a search point with id @a aSearchPointWithIdentifier and return a search point reflected in the plane
    /// with the same identifier
    [[nodiscard]] auto reflect(
        const third_party_integration::stk_search::SearchPointWithIdentifier& aSearchPointWithIdentifier) const
        -> third_party_integration::stk_search::SearchPointWithIdentifier;

    ///@brief Take a plane @a aPlane and reflect it through this plane
    [[nodiscard]] auto reflect(const Plane& aPlane) const -> Plane;
};

}  // namespace plato::filter::extension::kernel_filters

#endif
