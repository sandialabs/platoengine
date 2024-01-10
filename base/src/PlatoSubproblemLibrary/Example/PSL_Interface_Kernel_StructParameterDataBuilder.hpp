// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Implementation_Kernel_StructParameterData.hpp"
#include "PSL_Abstract_ParameterDataBuilder.hpp"

namespace PlatoSubproblemLibrary
{
class Abstract_ParameterData;

namespace example
{
struct Kernel_StructParameterData;

class Interface_Kernel_StructParameterDataBuilder : public AbstractInterface::ParameterDataBuilder
{
public:

    Interface_Kernel_StructParameterDataBuilder();
    ~Interface_Kernel_StructParameterDataBuilder() override;

    void set_data(example::Kernel_StructParameterData* data);
    ParameterData* build() override;

private:

    example::Kernel_StructParameterData* m_data;
};

}
}
