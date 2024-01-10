// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

#include "PSL_Abstract_OptimizationMesh.hpp"

#include <vector>
#include <cstddef>

namespace PlatoSubproblemLibrary
{
class Point;
namespace AbstractInterface
{
class OptimizationMesh;
}

namespace example
{

class MeshMonolithic;

class Interface_MeshMonolithic : public AbstractInterface::OptimizationMesh
{
public:

    Interface_MeshMonolithic();
    ~Interface_MeshMonolithic() override;

    void set_mesh(MeshMonolithic* mesh);

    // depreciated-ish
    virtual std::vector<size_t> get_point_neighbors(const size_t& index);

    // PointCloud
    Point get_point(const size_t& index) override;
    size_t get_num_points() override;

    // Mesh
    size_t get_num_blocks() override;
    size_t get_num_elements(size_t block_index) override;
    std::vector<size_t> get_nodes_from_element(size_t block_index, size_t element_index) override;

    // OptimizationMesh
    bool is_block_optimizable(size_t block_index) override;

private:

    MeshMonolithic* m_mesh;
};

}
}
