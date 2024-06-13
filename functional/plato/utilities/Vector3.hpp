#ifndef PLATO_UTILITIES_VECTOR3
#define PLATO_UTILITIES_VECTOR3

namespace plato::utilities
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

}  // namespace plato::utilities
#endif
