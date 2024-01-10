// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Abstract_ParallelVector.hpp"

#include <vector>
#include <cstddef>

namespace PlatoSubproblemLibrary
{
namespace example
{

class Interface_ParallelVector : public AbstractInterface::ParallelVector
{
public:
    Interface_ParallelVector(std::vector<double> data = std::vector<double>(0));
    ~Interface_ParallelVector() override;

    size_t get_length() override;
    double get_value(size_t index) override;
    void set_value(size_t index, double value) override;

    std::vector<double> m_data;

protected:
};

}
}
