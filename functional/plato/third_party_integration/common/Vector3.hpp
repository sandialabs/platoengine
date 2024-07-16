#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace plato::third_party_integration::common
{
struct Coordinate
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    friend std::ostream& operator<<(std::ostream& stream, const Coordinate& aContainer)
    {
        stream << aContainer.x << '\t' << aContainer.y << '\t' << aContainer.z;
        return stream;
    }
};

struct Vector3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

[[nodiscard]] constexpr Vector3 operator-(const Coordinate& p0, const Coordinate& p1)
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

[[nodiscard]] constexpr Coordinate operator+(const Coordinate& p0, const Coordinate& p1)
{
    return Coordinate{/*.x=*/p0.x + p1.x,
                      /*.y=*/p0.y + p1.y,
                      /*.z=*/p0.z + p1.z};
}

[[nodiscard]] constexpr Coordinate operator/(const Coordinate& p0, const double& divisor)
{
    return Coordinate{/*.x=*/p0.x / divisor,
                      /*.y=*/p0.y / divisor,
                      /*.z=*/p0.z / divisor};
}

[[nodiscard]] constexpr Coordinate operator*(const Coordinate& p0, const double& scale)
{
    return Coordinate{/*.x=*/p0.x * scale,
                      /*.y=*/p0.y * scale,
                      /*.z=*/p0.z * scale};
}

template <typename Container3>
[[nodiscard]] constexpr double magnitude(const Container3& aContainer)
{
    return std::sqrt(aContainer.x * aContainer.x + aContainer.y * aContainer.y + aContainer.z * aContainer.z);
}

template <typename Container3>
[[nodiscard]] std::vector<double> flatten(const Container3& aContainer3, const unsigned int aSpatialDimensions)
{
    return aSpatialDimensions == 2 ? std::vector<double>{aContainer3.x, aContainer3.y}
                                   : std::vector<double>{aContainer3.x, aContainer3.y, aContainer3.z};
}
}  // namespace plato::third_party_integration::common
#endif
