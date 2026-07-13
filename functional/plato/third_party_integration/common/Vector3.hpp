#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3

#include <algorithm>
#include <cmath>
#include <format>
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

/// @brief Output operator overload for a container3 type. Prints the entries of @a aContainer3 tab delimited to the
/// stream @a aStream.
template <Container3Type Container3>
auto operator<<(std::ostream& aStream, const Container3& aContainer3) -> std::ostream&
{
    aStream << aContainer3.x << '\t' << aContainer3.y << '\t' << aContainer3.z;
    return aStream;
}

/// @brief Convert @a aContainer3 to a fancy formatted string with precision @a aPrecision. The string will be in the
/// format (x,y,z) where the values have the precision specified. stream @a aStream.
template <Container3Type Container3>
[[nodiscard]] auto string(const Container3& aContainer3, unsigned int aPrecision) -> std::string
{
    return std::format("({:.{}f}, {:.{}f}, {:.{}f})", aContainer3.x, aPrecision, aContainer3.y, aPrecision,
                       aContainer3.z, aPrecision);
}

/// @brief Operator overload for subtraction, computes @a aLeftHandSide - @a aRightHandSide
template <Container3Type Container3>
[[nodiscard]] constexpr auto operator-(const Container3& aLeftHandSide, const Container3& aRightHandSide) -> Vector3
{
    return Vector3{.x = aLeftHandSide.x - aRightHandSide.x,
                   .y = aLeftHandSide.y - aRightHandSide.y,
                   .z = aLeftHandSide.z - aRightHandSide.z};
}

/// @brief Compute the dot product of @a aLeft and @a aRight
[[nodiscard]] constexpr auto dot(const Vector3& aLeft, const Vector3& aRight) -> double
{
    return aLeft.x * aRight.x + aLeft.y * aRight.y + aLeft.z * aRight.z;
}

/// @brief Compute the cross product of @a aLeft and @a aRight
[[nodiscard]] constexpr auto cross(const Vector3& aLeft, const Vector3& aRight) -> Vector3
{
    return Vector3{.x = aLeft.y * aRight.z - aLeft.z * aRight.y,
                   .y = aLeft.z * aRight.x - aLeft.x * aRight.z,
                   .z = aLeft.x * aRight.y - aLeft.y * aRight.x};
}

/// @brief Operator overload for addition, computes @a aLeftHandSide + @a aRightHandSide
template <Container3Type Container3>
[[nodiscard]] constexpr auto operator+(const Container3& aLeftHandSide, const Container3& aRightHandSide) -> Container3
{
    return Container3{.x = aLeftHandSide.x + aRightHandSide.x,
                      .y = aLeftHandSide.y + aRightHandSide.y,
                      .z = aLeftHandSide.z + aRightHandSide.z};
}

/// @brief Operator overload for addition of a Coordinate @a aCoordinate and a scalar @a aScalar
[[nodiscard]] constexpr auto operator+(const Coordinate& aCoordinate, const double aScalar) -> Coordinate
{
    return Coordinate{.x = aCoordinate.x + aScalar, .y = aCoordinate.y + aScalar, .z = aCoordinate.z + aScalar};
}

/// @brief Operator overload for component-wise division of a Container3 @a aNumerator a Container3 @a aDenominator
template <typename Container3>
[[nodiscard]] constexpr auto operator/(const Container3& aNumerator, const Container3& aDenominator) -> Container3
{
    return Container3{
        .x = aNumerator.x / aDenominator.x, .y = aNumerator.y / aDenominator.y, .z = aNumerator.z / aDenominator.z};
}

/// @brief Operator overload for component-wise division of a Container3 @a aContainer3 by a scalar @a aScalar
template <typename Container3>
[[nodiscard]] constexpr auto operator/(const Container3& aContainer3, const double& aDivisor) -> Container3
{
    return Container3{.x = aContainer3.x / aDivisor, .y = aContainer3.y / aDivisor, .z = aContainer3.z / aDivisor};
}

/// @brief Operator overload for component-wise multiplication of two Container3s @a aLeftHandSide and @a aRightHandSide
template <Container3Type Container3>
[[nodiscard]] constexpr auto operator*(const Container3& aLeftHandSide, const Container3& aRightHandSide) -> Container3
{
    return Container3{.x = aLeftHandSide.x * aRightHandSide.x,
                      .y = aLeftHandSide.y * aRightHandSide.y,
                      .z = aLeftHandSide.z * aRightHandSide.z};
}

/// @brief Operator overload for component-wise multiplication of a Container3 @a aContainer3 by a scalar @a aScalar
template <Container3Type Container3>
[[nodiscard]] constexpr auto operator*(const Container3& aContainer3, const double& aScalar) -> Container3
{
    return Container3{.x = aContainer3.x * aScalar, .y = aContainer3.y * aScalar, .z = aContainer3.z * aScalar};
}

/// @brief Operator overload for component-wise division of a scalar @a aScalar by Coordinate @a aCoordinate
[[nodiscard]] constexpr auto operator/(const double& aScalar, const Coordinate& aCoordinate) -> Coordinate
{
    return Coordinate{.x = aScalar / aCoordinate.x, .y = aScalar / aCoordinate.y, .z = aScalar / aCoordinate.z};
}

/// @brief Apply the floor operation to each component of @a aCoordinate
[[nodiscard]] constexpr auto floor(const Coordinate& aCoordinate) -> Coordinate
{
    return Coordinate{std::floor(aCoordinate.x), std::floor(aCoordinate.y), std::floor(aCoordinate.z)};
}

/// @brief Determine the magnitude of a Container3 @a aContainer3
template <Container3Type Container3>
[[nodiscard]] constexpr auto magnitude(const Container3& aContainer3) -> double
{
    return std::sqrt(aContainer3.x * aContainer3.x + aContainer3.y * aContainer3.y + aContainer3.z * aContainer3.z);
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

/// @brief Normalize the Container3 @a aContainer3
template <Container3Type Container3>
constexpr void normalize(Container3& aContainer3)
{
    const double tMagnitude = magnitude(aContainer3);
    if (std::fabs(tMagnitude) < std::numeric_limits<double>::min())
    {
        throw std::invalid_argument("Attempting to normalize a vector with length 0.");
    }
    aContainer3 = aContainer3 / tMagnitude;
}

inline UnitVector3::UnitVector3(double aXComponent, double aYComponent, double aZComponent)
    : mVector({aXComponent, aYComponent, aZComponent})
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
