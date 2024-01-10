// PlatoSubproblemLibraryVersion(6): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Interface_ParallelExchanger_global.hpp"

#include <vector>
#include <cstddef>

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class ParallelVector;
}
class AbstractAuthority;

namespace example
{

class Interface_ParallelExchanger_ownershipAndGlobal : public Interface_ParallelExchanger_global
{
public:
    Interface_ParallelExchanger_ownershipAndGlobal(AbstractAuthority* authority);
    ~Interface_ParallelExchanger_ownershipAndGlobal() override;

    // put ownership
    void put_is_owned(const std::vector<bool>& is_owned);

protected:

    void build_node_ownership(std::vector<int>& node_ownership) override;

    // received data
    std::vector<bool> m_is_owned;

};

}
}
