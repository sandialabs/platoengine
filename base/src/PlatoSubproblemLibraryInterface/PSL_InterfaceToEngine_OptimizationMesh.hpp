#pragma once

#include "PSL_Abstract_OptimizationMesh.hpp"

#include <cstddef>

namespace PlatoSubproblemLibrary
{
class Point;
}
class DataMesh;

namespace Plato
{

class InterfaceToEngine_OptimizationMesh : public PlatoSubproblemLibrary::AbstractInterface::OptimizationMesh
{
public:
    InterfaceToEngine_OptimizationMesh(DataMesh* mesh);
    ~InterfaceToEngine_OptimizationMesh() override;

    PlatoSubproblemLibrary::Point get_point(const size_t& index) override;
    size_t get_num_points() override;

    size_t get_num_blocks() override;
    size_t get_num_elements(size_t block_index) override;
    std::vector<size_t> get_nodes_from_element(size_t block_index, size_t element_index) override;

    bool is_block_optimizable(size_t block_index) override;

private:
    DataMesh* m_mesh;
};

}
