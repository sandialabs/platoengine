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

/// @brief Unit vector class. Always guaranteed to contain
/// a normalized vector. If construction would result in a normalized
/// vector of length 0 an exception is thrown (this will be caught in
/// the normalize() function that takes a Vector3 as input).
class UnitVector3
{
   public:
    UnitVector3(const double aX, const double aY, const double aZ);
    UnitVector3(const Vector3& aVector);

    operator Vector3() const;

   private:
    Vector3 mVector;
};

template <typename T>
concept Container3Type = std::is_same_v<T, Coordinate> || std::is_same_v<T, Vector3>;

inline auto operator<<(std::ostream& stream, const Coordinate& aContainer) -> std::ostream&
{
    stream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
    return stream;
}

inline auto operator<<(std::ostream& stream, const Vector3& aContainer) -> std::ostream&
{
    stream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
    return stream;
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator-(const Container3& p0, const Container3& p1) -> Vector3
{
    return Vector3{.x = p0.x - p1.x, .y = p0.y - p1.y, .z = p0.z - p1.z};
}

[[nodiscard]] constexpr auto dot(const Vector3& a, const Vector3& b) -> double
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] constexpr auto cross(const Vector3& a, const Vector3& b) -> Vector3
{
    return Vector3{.x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator+(const Container3& p0, const Container3& p1) -> Container3
{
    return Container3{.x = p0.x + p1.x, .y = p0.y + p1.y, .z = p0.z + p1.z};
}

[[nodiscard]] constexpr auto operator+(const Coordinate& p0, const double scalar) -> Coordinate
{
    return Coordinate{.x = p0.x + scalar, .y = p0.y + scalar, .z = p0.z + scalar};
}

[[nodiscard]] constexpr auto operator/(const Coordinate& p0, const Coordinate& divisor) -> Coordinate
{
    return Coordinate{.x = p0.x / divisor.x, .y = p0.y / divisor.y, .z = p0.z / divisor.z};
}

template <typename Container3>
[[nodiscard]] constexpr auto operator/(const Container3& p0, const double& divisor) -> Container3
{
    return Container3{.x = p0.x / divisor, .y = p0.y / divisor, .z = p0.z / divisor};
}

template <Container3Type Container3>
[[nodiscard]] constexpr auto operator*(const Container3& p0, const double& scale) -> Container3
{
    return Container3{.x = p0.x * scale, .y = p0.y * scale, .z = p0.z * scale};
}

[[nodiscard]] constexpr auto operator*(const Coordinate& p0, const Coordinate& p1) -> Coordinate
{
    return Coordinate{.x = p0.x * p1.x, .y = p0.y * p1.y, .z = p0.z * p1.z};
}

[[nodiscard]] constexpr auto operator/(const double& scalar, const Coordinate& p0) -> Coordinate
{
    return Coordinate{.x = scalar / p0.x, .y = scalar / p0.y, .z = scalar / p0.z};
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
[[nodiscard]] constexpr auto minimum_entries(const Container3& aContainerOne,
                                             const Container3& aContainerTwo) -> Container3
{
    return Container3{std::min(aContainerOne.x, aContainerTwo.x), std::min(aContainerOne.y, aContainerTwo.y),
                      std::min(aContainerOne.z, aContainerTwo.z)};
}

/// @brief Return the componentwise maximum values in the two Container3s @a aContainerOne and @a aContainerTwo
template <Container3Type Container3>
[[nodiscard]] constexpr auto maximum_entries(const Container3& aContainerOne,
                                             const Container3& aContainerTwo) -> Container3
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

inline UnitVector3::UnitVector3(const double aX, const double aY, const double aZ) : mVector{aX, aY, aZ}
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
