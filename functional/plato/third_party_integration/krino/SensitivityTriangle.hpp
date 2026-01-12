#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYTRIANGLE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYTRIANGLE

#include <cstdint>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::krino
{

/// @brief Enumeraton for the 3 components (x,y,z) of
// the traingle's normal derivative.
enum NormalSensitivityComponent : std::uint8_t
{
    kDNormalDNodeXCoord,
    kDNormalDNodeYCoord,
    kDNormalDNodeZCoord
};

/// @brief Pairing of node id and coords for easy access.
using GlobalNodeID = size_t;
using NodeIDCoordsPair = std::pair<GlobalNodeID, common::Coordinate>;

/// @brief Generic sensitivity with 3 components.
using Sensitivity = common::Vector3;

/// @brief Grouping of node id and node sensitivity.
using NodeGradient = std::pair<GlobalNodeID, Sensitivity>;
/// @brief Grouping of sensitivities of nodes in a triangle.
using TriangleGradient = std::array<NodeGradient, 3>;

/// @brief Sensitivity of the triangle area for single node.
using AreaSensitivityWRTNodalCoordinates = Sensitivity;
/// @brief Grouping of nodal area sensitivities for a given triangle.
using TriangleAreaSensitivity = std::array<AreaSensitivityWRTNodalCoordinates, 3>;

/// @brief Sensitivity of the triangle normal to a single node coordinate.
using DNormalDNodeCoordinate = Sensitivity;
/// @brief Sensitivities of the triangle normal to the 3 coordinates of a single node in the triangle.
using NormalSensitivityWRTNodalCoordinates = std::array<DNormalDNodeCoordinate, 3>;

/// @brief Grouping of nodal normal sensitivities for a given triangle.
using TriangleNormalSensitivity = std::array<NormalSensitivityWRTNodalCoordinates, 3>;

/// @brief This triangle data structure is used during the calculations of the change
/// of the triangle's area and normal wrt changes in the nodal coordinates.
struct SensitivityTriangle
{
    std::array<NodeIDCoordsPair, 3> mNodes;

    [[nodiscard]] common::UnitVector3 normal() const;
    [[nodiscard]] double area() const;
};

}  // namespace plato::third_party_integration::krino
#endif
