#ifndef PLATO_MESH_MESHFIELDWRITER
#define PLATO_MESH_MESHFIELDWRITER

#include <filesystem>
#include <string_view>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::design_variables
{
struct MeshDesignVariables;
}

namespace plato::mesh
{
/// @brief A mixin class for Mesh that provides functions to write a design variable field to disk.
class MeshFieldWriter : public Mesh
{
   public:
    explicit MeshFieldWriter(Mesh aMeshBase);

    /// @brief Writes the scalar design variable field in @a aScalarField to the file at path @a aFilePath
    /// assuming that the field represents a nodal field.
    ///
    /// Any values associated with fixed blocks in @a aScalarField are assigned to @a aFixedValue.
    /// @warning If a file exists at path @a aOutputPath, the file is overwritten.
    void writeNodalField(const std::filesystem::path& aOutputPath,
                         const NodalFieldVectorReference& aScalarField,
                         std::string_view aFieldName,
                         double aFixedValue) const;

    /// @brief Writes the scalar design variable field in @a aScalarField to the file at path @a aFilePath
    /// assuming that the field represents a nodal field.
    ///
    /// Any values associated with fixed blocks in @a aScalarField are assigned to @a aFixedValue.
    /// @warning If a file exists at path @a aOutputPath, the file is overwritten.
    void writeElementField(const std::filesystem::path& aFilePath,
                           const ElementFieldVectorReference& aScalarField,
                           std::string_view aFieldName,
                           double aFixedValue) const;

    /// @brief Writes the design variables in @a aMeshDesignVariables to the file with name @a aFilePath.
    ///
    /// The type of field (node vs. element) is determined from the number of design variables.
    /// @warning If a file exists at path @a aOutputPath, the file is overwritten.
    void writeMeshDesignVariables(const std::filesystem::path& aFilePath,
                                  const design_variables::MeshDesignVariables& aMeshDesignVariables,
                                  std::string_view aFieldName,
                                  double aFixedValue) const;
};
}  // namespace plato::mesh

#endif
