#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3

#include <algorithm>
#include <cmath>
#include <iostream>
#include <ranges>
#include <vector>

namespace plato::third_party_integration::common
{

// This is part of platoengine's API, so the names must stay the same for compatibility.
// NOLINTBEGIN(readability-identifier-naming)
struct Coordinate
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    auto operator==(const Coordinate& aCoordinate) const -> bool = default;
};

struct Vector3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    auto operator==(const Vector3& aVector) const -> bool = default;
    auto operator+=(const Vector3& aVector) -> Vector3&;
};
// NOLINTEND(readability-identifier-naming)

/// @brief Unit vector class. Always guaranteed to contain
/// a normalized vector. If construction would result in a normalized
/// vector of length 0 an exception is thrown (this will be caught in
/// the normalize() function that takes a Vector3 as input).
class UnitVector3
{
   public:
    UnitVector3(double aXComponent, double aYComponent, double aZComponent);
    UnitVector3(const Vector3& aVector);

    operator Vector3() const;

   private:
    Vector3 mVector;
};

template <typename T>
concept Container3Type = std::is_same_v<T, Coordinate> || std::is_same_v<T, Vector3>;

inline auto operator<<(std::ostream& aStream, const Coordinate& aContainer) -> std::ostream&
{
    aStream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
    return aStream;
}

inline auto operator<<(std::ostream& aStream, const Vector3& aContainer) -> std::ostream&
{
    aStream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
    return aStream;
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator-(const Container3& aP0, const Container3& aP1) -> Vector3
{
    return Vector3{.x = aP0.x - aP1.x, .y = aP0.y - aP1.y, .z = aP0.z - aP1.z};
}

[[nodiscard]] constexpr auto dot(const Vector3& aLeft, const Vector3& aRight) -> double
{
    return aLeft.x * aRight.x + aLeft.y * aRight.y + aLeft.z * aRight.z;
}

[[nodiscard]] constexpr auto cross(const Vector3& aLeft, const Vector3& aRight) -> Vector3
{
    return Vector3{.x = aLeft.y * aRight.z - aLeft.z * aRight.y,
                   .y = aLeft.z * aRight.x - aLeft.x * aRight.z,
                   .z = aLeft.x * aRight.y - aLeft.y * aRight.x};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator+(const Container3& aP0, const Container3& aP1) -> Container3
{
    return Container3{.x = aP0.x + aP1.x, .y = aP0.y + aP1.y, .z = aP0.z + aP1.z};
}

[[nodiscard]] constexpr auto operator+(const Coordinate& aP0, const double aScalar) -> Coordinate
{
    return Coordinate{.x = aP0.x + aScalar, .y = aP0.y + aScalar, .z = aP0.z + aScalar};
}

[[nodiscard]] constexpr auto operator/(const Coordinate& aP0, const Coordinate& aDivisor) -> Coordinate
{
    return Coordinate{.x = aP0.x / aDivisor.x, .y = aP0.y / aDivisor.y, .z = aP0.z / aDivisor.z};
}

template <typename Container3>
[[nodiscard]] constexpr auto operator/(const Container3& aP0, const double& aDivisor) -> Container3
{
    return Container3{.x = aP0.x / aDivisor, .y = aP0.y / aDivisor, .z = aP0.z / aDivisor};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator*(const Container3& aP0, const double& aScale) -> Container3
{
    return Container3{.x = aP0.x * aScale, .y = aP0.y * aScale, .z = aP0.z * aScale};
}

[[nodiscard]] constexpr auto operator*(const Coordinate& aP0, const Coordinate& aP1) -> Coordinate
{
    return Coordinate{.x = aP0.x * aP1.x, .y = aP0.y * aP1.y, .z = aP0.z * aP1.z};
}

[[nodiscard]] constexpr auto operator/(const double& aScalar, const Coordinate& aP0) -> Coordinate
{
    return Coordinate{.x = aScalar / aP0.x, .y = aScalar / aP0.y, .z = aScalar / aP0.z};
}

[[nodiscard]] constexpr auto floor(const Coordinate& aInputCoord) -> Coordinate
{
    return Coordinate{std::floor(aInputCoord.x), std::floor(aInputCoord.y), std::floor(aInputCoord.z)};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto magnitude(const Container3& aContainer) -> double
{
    return std::sqrt(aContainer.x * aContainer.x + aContainer.y * aContainer.y + aContainer.z * aContainer.z);
}

/// @brief Return the componentwise minimum values in the two Container3s @a aContainerOne and @a aContainerTwo
template <Container3Type Container3>
[[nodiscard]] constexpr auto minimum_entries(const Container3& aContainerOne, const Container3& aContainerTwo)
    -> Container3
{
    return Container3{std::min(aContainerOne.x, aContainerTwo.x), std::min(aContainerOne.y, aContainerTwo.y),
                      std::min(aContainerOne.z, aContainerTwo.z)};
}

/// @brief Return the componentwise maximum values in the two Container3s @a aContainerOne and @a aContainerTwo
template <Container3Type Container3>
[[nodiscard]] constexpr auto maximum_entries(const Container3& aContainerOne, const Container3& aContainerTwo)
    -> Container3
{
    return Container3{std::max(aContainerOne.x, aContainerTwo.x), std::max(aContainerOne.y, aContainerTwo.y),
                      std::max(aContainerOne.z, aContainerTwo.z)};
}

inline void normalize(Vector3& aVector)
{
    const double tMagnitude = magnitude(aVector);
    if (std::fabs(tMagnitude) < std::numeric_limits<double>::min())
    {
        throw std::invalid_argument("Attempting to normalize a vector with length 0.");
    }
    aVector = aVector * (1.0 / tMagnitude);
}

inline UnitVector3::UnitVector3(const double aXComponent, const double aYComponent, const double aZComponent)
    : mVector{aXComponent, aYComponent, aZComponent}
{
    normalize(mVector);
}

inline UnitVector3::UnitVector3(const Vector3& aVector) : mVector(aVector) { normalize(mVector); }

inline UnitVector3::operator Vector3() const { return mVector; }

inline auto Vector3::operator+=(const Vector3& aVector) -> Vector3&
{
    x += aVector.x;
    y += aVector.y;
    z += aVector.z;
    return *this;
}

/// @brief Take a std::vector of Container3 @a aContainerVector and return a flattened vector of values
/// (x0,y0,z0,x1,y1,z1...xN-1,yN-1,zN-1)
template <Container3Type Container3>
[[nodiscard]] auto flatten(const std::vector<Container3>& aContainerVector) -> std::vector<double>
{
    auto tFlattenedView = aContainerVector |
                          std::views::transform([](const auto& aContainer3)
                                                { return std::array{aContainer3.x, aContainer3.y, aContainer3.z}; }) |
                          std::views::join | std::views::common;

    return std::vector(tFlattenedView.begin(), tFlattenedView.end());
}

}  // namespace plato::third_party_integration::common
#endif
