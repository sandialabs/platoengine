#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <ranges>
#include <vector>

namespace plato::third_party_integration::common
{

struct Coordinate
{
    double mX = 0.0;
    double mY = 0.0;
    double mZ = 0.0;
    auto operator==(const Coordinate& aCoordinate) const -> bool = default;
};

struct Vector3
{
    double mX = 0.0;
    double mY = 0.0;
    double mZ = 0.0;
    auto operator==(const Vector3& aVector) const -> bool = default;
    auto operator+=(const Vector3& aVector) -> Vector3&;
};

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
    aStream << aContainer.mX << '\t' << aContainer.mY << '\t' << aContainer.mZ;
    return aStream;
}

inline auto operator<<(std::ostream& aStream, const Vector3& aContainer) -> std::ostream&
{
    aStream << aContainer.mX << '\t' << aContainer.mY << '\t' << aContainer.mZ;
    return aStream;
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator-(const Container3& aP0, const Container3& aP1) -> Vector3
{
    return Vector3{.mX = aP0.mX - aP1.mX, .mY = aP0.mY - aP1.mY, .mZ = aP0.mZ - aP1.mZ};
}

[[nodiscard]] constexpr auto dot(const Vector3& aLeft, const Vector3& aRight) -> double
{
    return aLeft.mX * aRight.mX + aLeft.mY * aRight.mY + aLeft.mZ * aRight.mZ;
}

[[nodiscard]] constexpr auto cross(const Vector3& aLeft, const Vector3& aRight) -> Vector3
{
    return Vector3{.mX = aLeft.mY * aRight.mZ - aLeft.mZ * aRight.mY,
                   .mY = aLeft.mZ * aRight.mX - aLeft.mX * aRight.mZ,
                   .mZ = aLeft.mX * aRight.mY - aLeft.mY * aRight.mX};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator+(const Container3& aP0, const Container3& aP1) -> Container3
{
    return Container3{.mX = aP0.mX + aP1.mX, .mY = aP0.mY + aP1.mY, .mZ = aP0.mZ + aP1.mZ};
}

[[nodiscard]] constexpr auto operator+(const Coordinate& aP0, const double aScalar) -> Coordinate
{
    return Coordinate{.mX = aP0.mX + aScalar, .mY = aP0.mY + aScalar, .mZ = aP0.mZ + aScalar};
}

[[nodiscard]] constexpr auto operator/(const Coordinate& aP0, const Coordinate& aDivisor) -> Coordinate
{
    return Coordinate{.mX = aP0.mX / aDivisor.mX, .mY = aP0.mY / aDivisor.mY, .mZ = aP0.mZ / aDivisor.mZ};
}

template <typename Container3>
[[nodiscard]] constexpr auto operator/(const Container3& aP0, const double& aDivisor) -> Container3
{
    return Container3{.mX = aP0.mX / aDivisor, .mY = aP0.mY / aDivisor, .mZ = aP0.mZ / aDivisor};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator*(const Container3& aP0, const double& aScale) -> Container3
{
    return Container3{.mX = aP0.mX * aScale, .mY = aP0.mY * aScale, .mZ = aP0.mZ * aScale};
}

[[nodiscard]] constexpr auto operator*(const Coordinate& aP0, const Coordinate& aP1) -> Coordinate
{
    return Coordinate{.mX = aP0.mX * aP1.mX, .mY = aP0.mY * aP1.mY, .mZ = aP0.mZ * aP1.mZ};
}

[[nodiscard]] constexpr auto operator/(const double& aScalar, const Coordinate& aP0) -> Coordinate
{
    return Coordinate{.mX = aScalar / aP0.mX, .mY = aScalar / aP0.mY, .mZ = aScalar / aP0.mZ};
}

[[nodiscard]] constexpr auto floor(const Coordinate& aInputCoord) -> Coordinate
{
    return Coordinate{std::floor(aInputCoord.mX), std::floor(aInputCoord.mY), std::floor(aInputCoord.mZ)};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto magnitude(const Container3& aContainer) -> double
{
    return std::sqrt(aContainer.mX * aContainer.mX + aContainer.mY * aContainer.mY + aContainer.mZ * aContainer.mZ);
}

/// @brief Return the componentwise minimum values in the two Container3s @a aContainerOne and @a aContainerTwo
template <Container3Type Container3>
[[nodiscard]] constexpr auto minimum_entries(const Container3& aContainerOne,
                                             const Container3& aContainerTwo) -> Container3
{
    return Container3{std::min(aContainerOne.mX, aContainerTwo.mX), std::min(aContainerOne.mY, aContainerTwo.mY),
                      std::min(aContainerOne.mZ, aContainerTwo.mZ)};
}

/// @brief Return the componentwise maximum values in the two Container3s @a aContainerOne and @a aContainerTwo
template <Container3Type Container3>
[[nodiscard]] constexpr auto maximum_entries(const Container3& aContainerOne,
                                             const Container3& aContainerTwo) -> Container3
{
    return Container3{std::max(aContainerOne.mX, aContainerTwo.mX), std::max(aContainerOne.mY, aContainerTwo.mY),
                      std::max(aContainerOne.mZ, aContainerTwo.mZ)};
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
    mX += aVector.mX;
    mY += aVector.mY;
    mZ += aVector.mZ;
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
