#ifndef PLATO_MESH_MESHPROXY
#define PLATO_MESH_MESHPROXY

#include <filesystem>
#include <map>
#include <vector>

namespace plato::mesh
{
/// @brief Struct used for pairing a density value with a global mesh id.
struct Density
{
    using IndexType = std::size_t;

    IndexType mGlobalID = 0;
    double mDensity = 0.0;
};

/// @brief Represents a mesh on disk as well as a nodal density field.
///
/// This object is used as an argument to most objectives to represent a mesh
/// and a density field.
struct MeshProxy
{
    using DensityVector = std::vector<Density>;
    using BlockDensities = std::map<unsigned int, DensityVector>;

    std::filesystem::path mFileName;
    BlockDensities mBlockDensities;
};

}  // namespace plato::mesh

#endif
