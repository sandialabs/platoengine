#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_TRIANGLE
#define PLATO_THIRDPARTYINTEGRATION_STKIO_TRIANGLE

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{
struct Triangle
{
    constexpr static auto kNumVertices = unsigned{3};
    using Indices = std::array<unsigned int, kNumVertices>;
    using VertexSensitivities = std::array<common::Vector3, kNumVertices>;

    common::Coordinate p0;
    common::Coordinate p1;
    common::Coordinate p2;

    [[nodiscard]] auto volume() const -> double;
    [[nodiscard]] auto normal() const -> common::Vector3;
    [[nodiscard]] auto centroid() const -> common::Coordinate;
    [[nodiscard]] auto volumeVertexSensitivities() const -> VertexSensitivities;

   private:
    [[nodiscard]] auto areaVector() const -> common::Vector3;
};

}  // namespace plato::third_party_integration::stk_io
#endif
