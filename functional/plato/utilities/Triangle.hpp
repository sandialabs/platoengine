#ifndef PLATO_UTILITIES_TRIANGLE
#define PLATO_UTILITIES_TRIANGLE

#include <array>

#include "plato/utilities/Vector3.hpp"

namespace plato::utilities
{
struct Triangle
{
    Coordinate p0;
    Coordinate p1;
    Coordinate p2;

    [[nodiscard]] double volume() const;

    constexpr static auto kNumVertices = unsigned{3};
    using Indices = std::array<unsigned int, kNumVertices>;
};

}  // namespace plato::utilities
#endif
