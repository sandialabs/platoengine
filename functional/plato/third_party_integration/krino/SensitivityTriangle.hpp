#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYTRIANGLE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYTRIANGLE

#include <cstdint>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::krino
{

enum NormalSensitivityComponent : std::uint8_t
{
    kDNormalDNodeXCoord,
    kDNormalDNodeYCoord,
    kDNormalDNodeZCoord
};

using GlobalNodeID = size_t;
using NodeIDCoordsPair = std::pair<GlobalNodeID, common::Coordinate>;

using Sensitivity = common::Vector3;
using NodeGradient = std::pair<GlobalNodeID, Sensitivity>;
using TriangleGradient = std::array<NodeGradient, 3>;

using DAreaDNode = common::Vector3;
using AreaSensitivityWRTNodalCoordinates = Sensitivity;
using TriangleAreaSensitivity = std::array<AreaSensitivityWRTNodalCoordinates, 3>;

using DNormalDNodeCoordinate = Sensitivity;
using NormalSensitivityWRTNodalCoordinates = std::array<DNormalDNodeCoordinate, 3>;

using TriangleNormalSensitivity = std::array<NormalSensitivityWRTNodalCoordinates, 3>;

struct SensitivityTriangle
{
    std::array<NodeIDCoordsPair, 3> mNodes;

    [[nodiscard]] common::UnitVector3 normal() const;
    [[nodiscard]] double area() const;
};

}  // namespace plato::third_party_integration::krino
#endif
