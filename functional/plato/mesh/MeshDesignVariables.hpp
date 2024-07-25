#ifndef PLATO_MESH_MESHDESIGNVARIABLES
#define PLATO_MESH_MESHDESIGNVARIABLES

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

/// @brief Contains mesh design variables (nodal or element density fields) organized by mesh block.
///
/// This object is used as an argument to most objectives to represent a mesh and a density field.
struct MeshDesignVariables
{
    using BlockIDType = unsigned;
    using DensityVector = std::vector<Density>;
    using BlockDensities = std::map<BlockIDType, DensityVector>;

    std::filesystem::path mFileName;
    BlockDensities mBlockDensities;
};

}  // namespace plato::mesh

#endif
