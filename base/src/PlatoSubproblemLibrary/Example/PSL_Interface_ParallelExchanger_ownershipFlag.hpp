// PlatoSubproblemLibraryVersion(6): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Abstract_ParallelExchanger_Managed.hpp"

#include <vector>
#include <cstddef>

namespace PlatoSubproblemLibrary
{
class AbstractAuthority;

namespace example
{

class Interface_ParallelExchanger_ownershipFlag : public AbstractInterface::ParallelExchanger_Managed
{
public:
    Interface_ParallelExchanger_ownershipFlag(AbstractAuthority* authority);
    ~Interface_ParallelExchanger_ownershipFlag() override;

    // put ownership
    void put_is_owned(const std::vector<bool>& is_owned);

    void build() override;

protected:

    // received data
    std::vector<bool> m_is_owned;

};

}
}
