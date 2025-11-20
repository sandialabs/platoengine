#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYTRIANGLE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SENSITIVITYTRIANGLE

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::krino
{

struct SensitivityNode
{
    size_t mGlobalNodeID;
    common::Coordinate mCoordinates;
    common::Vector3 mAreaSensitivity;
    std::array<common::Vector3, 3> mNormalSensitivity;
};

struct SensitivityTriangle
{
    SensitivityNode mNode1, mNode2, mNode3;

    [[nodiscard]] common::UnitVector3 normal() const;
    [[nodiscard]] double area() const;
};

}  // namespace plato::third_party_integration::krino
#endif
