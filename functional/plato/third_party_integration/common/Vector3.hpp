#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

namespace plato::third_party_integration::common
{
struct Coordinate
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Vector3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

inline std::ostream& operator<<(std::ostream& stream, const Coordinate& aContainer)
{
    stream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const Vector3& aContainer)
{
    stream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
    return stream;
}

/// @todo Replace with `== default` in c++20
[[nodiscard]] inline constexpr bool operator==(const Coordinate& aLHS, const Coordinate& aRHS)
{
    return aLHS.x == aRHS.x && aLHS.y == aRHS.y && aLHS.z == aRHS.z;
}

template <typename Container3>
[[nodiscard]] constexpr Vector3 operator-(const Container3& p0, const Container3& p1)
{
    return Vector3{/*.x=*/p0.x - p1.x,
                   /*.y=*/p0.y - p1.y,
                   /*.z=*/p0.z - p1.z};
}

[[nodiscard]] constexpr double dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

[[nodiscard]] constexpr Vector3 cross(const Vector3& a, const Vector3& b)
{
    return Vector3{/*.x=*/a.y * b.z - a.z * b.y,
                   /*.y=*/a.z * b.x - a.x * b.z,
                   /*.z=*/a.x * b.y - a.y * b.x};
}

template <typename Container3>
[[nodiscard]] constexpr Container3 operator+(const Container3& p0, const Container3& p1)
{
    return Container3{/*.x=*/p0.x + p1.x,
                      /*.y=*/p0.y + p1.y,
                      /*.z=*/p0.z + p1.z};
}

[[nodiscard]] constexpr Coordinate operator+(const Coordinate& p0, const double scalar)
{
    return Coordinate{/*.x=*/p0.x + scalar,
                      /*.y=*/p0.y + scalar,
                      /*.z=*/p0.z + scalar};
}

[[nodiscard]] constexpr Coordinate operator/(const Coordinate& p0, const Coordinate& divisor)
{
    return Coordinate{/*.x=*/p0.x / divisor.x,
                      /*.y=*/p0.y / divisor.y,
                      /*.z=*/p0.z / divisor.z};
}

[[nodiscard]] constexpr Coordinate operator/(const Coordinate& p0, const double& divisor)
{
    return Coordinate{/*.x=*/p0.x / divisor,
                      /*.y=*/p0.y / divisor,
                      /*.z=*/p0.z / divisor};
}

template <typename Container3>
[[nodiscard]] constexpr Container3 operator*(const Container3& p0, const double& scale)
{
    return Container3{/*.x=*/p0.x * scale,
                      /*.y=*/p0.y * scale,
                      /*.z=*/p0.z * scale};
}

[[nodiscard]] constexpr Coordinate operator*(const Coordinate& p0, const Coordinate& p1)
{
    return Coordinate{/*.x=*/p0.x * p1.x,
                      /*.y=*/p0.y * p1.y,
                      /*.z=*/p0.z * p1.z};
}

[[nodiscard]] constexpr Coordinate operator/(const double& scalar, const Coordinate& p0)
{
    return Coordinate{/*.x=*/scalar / p0.x,
                      /*.y=*/scalar / p0.y,
                      /*.z=*/scalar / p0.z};
}

[[nodiscard]] constexpr Coordinate floor(const Coordinate& aInputCoord)
{
    return Coordinate{std::floor(aInputCoord.x), std::floor(aInputCoord.y), std::floor(aInputCoord.z)};
}

template <typename Container3>
[[nodiscard]] constexpr double magnitude(const Container3& aContainer)
{
    return std::sqrt(aContainer.x * aContainer.x + aContainer.y * aContainer.y + aContainer.z * aContainer.z);
}

}  // namespace plato::third_party_integration::common
#endif
