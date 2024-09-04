#ifndef PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLES
#define PLATO_DESIGN_VARIABLES_MESHDESIGNVARIABLES

#include <filesystem>
#include <map>
#include <vector>

namespace plato::design_variables
{
/// @brief Struct used for pairing a scalar design variable field value with a global mesh id and vector index.
struct ScalarFieldValue
{
    using IndexType = std::size_t;

    /// @brief The ID or index of the mesh entity (node or element) that this ScalarFieldValue associated with.
    IndexType mGlobalMeshEntityID = 0;
    /// @brief The index into the vector of design variables that this ScalarFieldValue associated with.
    /// This can be used to index into a linear array of design variables for converting between datatypes.
    IndexType mDesignVariableVectorIndex = 0;
    /// @brief Design variable value.
    double mValue = 0.0;
};

/// @brief Contains mesh design variables (nodal or element density fields) organized by mesh block.
///
/// This object is used as an argument to most objectives to represent a mesh and a density field.
struct MeshDesignVariables
{
    using BlockIDType = int64_t;
    using ScalarFieldVector = std::vector<ScalarFieldValue>;
    using BlockScalarField = std::map<BlockIDType, ScalarFieldVector>;

    std::filesystem::path mFileName;
    BlockScalarField mBlockScalarField;
};

}  // namespace plato::design_variables

#endif
