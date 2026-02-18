#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_RELFECTIONUTILITIES
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_RELFECTIONUTILITIES

#include <boost/mpi/communicator.hpp>
#include <ranges>
#include <vector>

#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/filter/extension/kernel_filters/Wedge.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::filter::extension::kernel_filters
{

template <typename... Planes>
concept PlanesType = (std::same_as<Planes, Plane> && ...);

/// @brief Take a point @a aPoint and mirror it through the wedge @a aWedge to find the shortest distance of the
/// test point against all the reflected points from the wedge
[[nodiscard]] auto minimum_distance_reflected_points_wedge(const SourcePoint& aPoint,
                                                           const TargetPoint& aTestPoint,
                                                           const Wedge& aWedge) -> double;

/// @brief Take a vector of points @a aPoints and a test point @a aTestPoint and compute the minimum distance of the
/// test point against all the reflected points from the list of planes @a planes
template <typename... Planes>
    requires PlanesType<Planes...>
[[nodiscard]] auto minimum_distance_reflected_points_planes_list(const SourcePoint& aPoint,
                                                                 const TargetPoint& aTestPoint,
                                                                 const Planes&... aPlanes) -> double;

namespace detail
{

/// @brief Take a vector of points @a aPoints and return a list of reflected points INCLUDING the originals after they
/// are reflected through a series of planes @a planes
template <typename PointType, typename... Planes>
[[nodiscard]] auto reflect_points_in_planes(std::vector<PointType> aPoints, const Planes&... aPlanes)
    -> std::vector<PointType>;

/// @brief Take a Plane @a aPlane and a vector of points @a aPoints and return a list of reflected points including the
/// originals
template <typename PointType>
[[nodiscard]] auto reflect_point_in_single_plane(std::vector<PointType> aPoints, const Plane& aPlane)
    -> std::vector<PointType>;

/// @brief Take a Plane @a aPlane and a vector of points @a aPoints and return a list of reflected points not including
/// the originals
template <typename PointType>
[[nodiscard]] auto reflect_point_in_single_plane_impl(std::vector<PointType> aPoints, const Plane& aPlane)
    -> std::vector<PointType>;

/// @brief Take a Wedge  @a aWedge, a vector of points @a aPoints and perform the mirroring of the points
/// sequentially in the wedge. Return a list of reflected points including the originals
///
/// The points are reflected in the first plane, then the second plane, then the first plane again, and so on. This
/// can achieve wedge symmetry if the correct number of reflections is picked, if it is too low it will
/// underpopulate the intended domain but not like what you might think in terms of rotations.
template <typename PointType>
[[nodiscard]] auto reflect_points_in_mirrored_wedge(std::vector<PointType> aPoints, const Wedge& aWedge)
    -> std::vector<PointType>;

/// @brief Compute the minimum distance between a point @a aTestPoint and the vector of points @a aSearchPoints
[[nodiscard]] auto minimum_distance_test_and_search_points(
    const third_party_integration::common::Coordinate& aTestPoint,
    const std::vector<third_party_integration::common::Coordinate>& aSearchPoints) -> double;

namespace
{

/// @brief This is a operator that allows for the composition of multiple planes and computing the mirroring that
/// occurs from a list of planes
template <typename PointType>
using ReflectionFunction = utilities::NamedType<std::function<std::vector<PointType>(const std::vector<PointType>&)>,
                                                struct ReflectionFunctionTag>;

template <typename PointType>
[[nodiscard]] auto operator|(ReflectionFunction<PointType> aInner, ReflectionFunction<PointType> aOuter)
    -> ReflectionFunction<PointType>
{
    return ReflectionFunction<PointType>{
        [mOuter = std::move(aOuter), mInner = std::move(aInner)](const std::vector<PointType>& aX)
        { return mOuter.mValue(mInner.mValue(aX)); }};
}

}  // namespace

template <typename PointType>
auto reflect_point_in_single_plane_impl(std::vector<PointType> aPoints, const Plane& aPlane) -> std::vector<PointType>
{
    std::transform(aPoints.begin(), aPoints.end(), aPoints.begin(),
                   [aPlane](const auto aCurrentPoint) -> PointType { return aPlane.reflect(aCurrentPoint); });
    return aPoints;
}

template <typename PointType>
auto reflect_point_in_single_plane(std::vector<PointType> aPoints, const Plane& aPlane) -> std::vector<PointType>
{
    auto tReflectedPoints = utilities::reserved_container<std::vector<PointType>>(aPoints.size() * 2);
    tReflectedPoints.insert(tReflectedPoints.end(), aPoints.begin(), aPoints.end());
    const auto tReflected = reflect_point_in_single_plane_impl(std::move(aPoints), aPlane);
    tReflectedPoints.insert(tReflectedPoints.end(), tReflected.begin(), tReflected.end());
    return tReflectedPoints;
}

template <typename PointType>
auto reflect_points_in_mirrored_wedge(std::vector<PointType> aPoints, const Wedge& aWedge) -> std::vector<PointType>
{
    const auto tPlanes = wedge_to_planes(aWedge);
    auto tReflectedPoints =
        utilities::reserved_container<std::vector<PointType>>((aWedge.mRepeats + 1) * aPoints.size());
    tReflectedPoints.insert(tReflectedPoints.end(), aPoints.begin(), aPoints.end());

    for (const auto& tPlane : tPlanes)
    {
        auto tLocalReflectedPoints = reflect_point_in_single_plane_impl(aPoints, tPlane);
        tReflectedPoints.insert(tReflectedPoints.end(), tLocalReflectedPoints.begin(), tLocalReflectedPoints.end());
        aPoints = std::move(tLocalReflectedPoints);
    }
    return tReflectedPoints;
}

template <typename PointType, typename... Planes>
    requires PlanesType<Planes...>
auto reflect_points_in_planes(std::vector<PointType> aPoints, const Planes&... aPlanes) -> std::vector<PointType>
{
    const auto tSingleReflectionLambda = [](const Plane& aPlane)
    {
        return ReflectionFunction<PointType>(
            [=](const std::vector<PointType>& aVector)
            { return detail::reflect_point_in_single_plane<PointType>(aVector, aPlane); });
    };
    const auto tComposed = (tSingleReflectionLambda(aPlanes) | ...);
    return tComposed.mValue(std::move(aPoints));
}

}  // namespace detail

template <typename... Planes>
    requires PlanesType<Planes...>
auto minimum_distance_reflected_points_planes_list(const SourcePoint& aPoint,
                                                   const TargetPoint& aTestPoint,
                                                   const Planes&... aPlanes) -> double
{
    const auto tSearchPoints =
        detail::reflect_points_in_planes<third_party_integration::common::Coordinate>({aPoint.mValue}, aPlanes...);
    return detail::minimum_distance_test_and_search_points(aTestPoint.mValue, tSearchPoints);
}

}  // namespace plato::filter::extension::kernel_filters

#endif
