#pragma once

#include "PSL_Implementation_NeuralNetwork_StructParameterData.hpp"
#include "PSL_Abstract_ParameterDataBuilder.hpp"

namespace PlatoSubproblemLibrary
{
class ParameterData;

namespace example
{
struct NeuralNetwork_StructParameterData;

class Interface_NeuralNetwork_StructParameterDataBuilder : public AbstractInterface::ParameterDataBuilder
{
public:

    Interface_NeuralNetwork_StructParameterDataBuilder();
    ~Interface_NeuralNetwork_StructParameterDataBuilder() override;

    void set_data(example::NeuralNetwork_StructParameterData* data);
    ParameterData* build() override;

private:

    example::NeuralNetwork_StructParameterData* m_data;
};

}
}
