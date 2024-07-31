#ifndef PLATO_MESH_MESHDESIGNVARIABLES
#define PLATO_MESH_MESHDESIGNVARIABLES

#include <filesystem>
#include <map>
#include <vector>

namespace plato::mesh
{
/// @brief Struct used for pairing a density value with a global mesh id and vector index.
struct Density
{
    using IndexType = std::size_t;

    /// @brief The ID or index of the mesh entity (node or element) that this Density associated with.
    IndexType mGlobalMeshEntityID = 0;
    /// @brief The index into the vector of design variables that this Density associated with.
    /// This can be used to index into a linear array of design variables for converting between datatypes.
    IndexType mDesignVariableVectorIndex = 0;
    /// @brief Density or design variable value.
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
