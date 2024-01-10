#pragma once

#include "PSL_Abstract_ParameterDataBuilder.hpp"
#include "Plato_InputData.hpp"

namespace PlatoSubproblemLibrary
{
class ParameterData;
}

namespace Plato
{

class InterfaceToEngine_ParameterDataBuilder : public PlatoSubproblemLibrary::AbstractInterface::ParameterDataBuilder
{
public:
    InterfaceToEngine_ParameterDataBuilder(InputData interface);
    ~InterfaceToEngine_ParameterDataBuilder() override;

    PlatoSubproblemLibrary::ParameterData* build() override;

private:
    Plato::InputData m_inputData;

};

}
