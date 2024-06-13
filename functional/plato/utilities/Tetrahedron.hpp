#ifndef PLATO_UTILITIES_TETRAHEDRON
#define PLATO_UTILITIES_TETRAHEDRON

#include <array>

#include "plato/utilities/Vector3.hpp"
namespace plato::utilities
{

struct Tetrahedron
{
    Coordinate p0;
    Coordinate p1;
    Coordinate p2;
    Coordinate p3;

    [[nodiscard]] double volume() const;

    constexpr static auto kNumVertices = unsigned{4};
    using Indices = std::array<unsigned int, kNumVertices>;
};

}  // namespace plato::utilities
#endif
