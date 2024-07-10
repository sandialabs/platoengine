#ifndef PLATO_MESH_MESHPROXY
#define PLATO_MESH_MESHPROXY

#include <filesystem>
#include <string_view>
#include <vector>

namespace plato::mesh
{

/// @brief Represents a mesh on disk as well as a nodal density field.
///
/// This object is used as an argument to most objectives to represent a mesh
/// and a density field.
struct MeshProxy
{
    std::filesystem::path mFileName;
    std::vector<double> mNodalDensities;
};

}  // namespace plato::mesh

#endif
