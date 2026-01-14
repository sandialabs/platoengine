#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"

#include <iterator>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::filter::extension::kernel_filters
{

auto minimum_distance_reflected_points_wedge(const SourcePoint& aPoint,
                                             const TargetPoint& aTestPoint,
                                             const Wedge& aWedge) -> double
{
    const auto tSearchPoints =
        detail::reflect_points_in_mirrored_wedge<third_party_integration::common::Coordinate>({aPoint.mValue}, aWedge);
    return detail::minimum_distance_test_and_search_points(aTestPoint.mValue, tSearchPoints);
}

namespace detail
{
auto minimum_distance_test_and_search_points(
    const third_party_integration::common::Coordinate& aTestPoint,
    const std::vector<third_party_integration::common::Coordinate>& aSearchPoints) -> double
{
    const auto tDistances =
        aSearchPoints |
        std::views::transform([aTestPoint](const auto& aSearchPoint)
                              { return third_party_integration::common::magnitude(aSearchPoint - aTestPoint); });
    return *std::ranges::min_element(tDistances);
}

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
