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

inline Vector3& Vector3::operator+=(const Vector3& aVector)
{
    x += aVector.x;
    y += aVector.y;
    z += aVector.z;
    return *this;
}

}  // namespace plato::third_party_integration::common
#endif
