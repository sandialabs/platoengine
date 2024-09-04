#ifndef PLATO_MESH_DESIGNVARIABLECONVERSION
#define PLATO_MESH_DESIGNVARIABLECONVERSION

#include <vector>

#include "plato/design_variables/MeshDesignVariables.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/NamedReference.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
using NodalFieldVector = utilities::NamedType<std::vector<double>, struct NodalFieldVectorTag>;
using ElementFieldVector = utilities::NamedType<std::vector<double>, struct ElementFieldVectorTag>;

using NodalFieldVectorReference =
    utilities::NamedReference<const std::vector<double>, struct NodalFieldVectorReferenceTag>;
using ElementFieldVectorReference =
    utilities::NamedReference<const std::vector<double>, struct ElementFieldVectorReferenceTag>;

struct DesignVariablesConversion : public Mesh
{
    explicit DesignVariablesConversion(Mesh aMesh);

    /// @brief Maps the nodal scalar field in @a aScalarField to a design_variables::MeshDesignVariables object using
    /// the node, element, and block info in the mesh.
    ///
    /// The ordering of @a aScalarField is assumed to match that of the vector returned by
    /// meshDesignVariablesToNodalFieldVector.
    /// @pre The size of @a aScalarField must be equal to the total number of nodes in the design domain of @a aMesh,
    /// which is given by numberOfDesignDomainElements in EntityCounts.
    design_variables::MeshDesignVariables nodalFieldToMeshDesignVariables(NodalFieldVectorReference aScalarField) const;

    /// @brief Maps the element scalar field in @a aScalarField to a design_variables::MeshDesignVariables object using
    /// the node, element, and block info in the mesh.
    /// @pre The size of @a aScalarField must be equal to the total number of elements in the design domain of @a aMesh,
    /// which is given by numberOfDesignDomainElements in EntityCounts.
    design_variables::MeshDesignVariables elementFieldToMeshDesignVariables(
        ElementFieldVectorReference aScalarField) const;

    /// @brief Maps the nodal scalar field contained in @a aMeshDesignVariables to a vector.
    ///
    /// The ordering of the resulting vector will be sorted by global node ID. For example if @a aMeshDesignVariables
    /// has the data (with the form {block_id : [[global_id, scalar], [global_id, scalar]]}):
    /// @verbatim
    /// { 1 : [[1, 0], [4, 1], [5, 0.5]], 2 : [[2, 1], [3, 0], [6, 0.5]]}
    /// @endverbatim
    /// The result will be:
    /// @verbatim
    /// [0, 1, 0, 1, 0.5, 0.5]
    /// @endverbatim
    NodalFieldVector meshDesignVariablesToNodalFieldVector(
        const design_variables::MeshDesignVariables& aMeshDesignVariables) const;

    /// @brief Maps the nodal scalar field contained in @a aMeshDesignVariables to a vector.
    ///
    /// The ordering of the resulting vector will be sorted by global element ID. See
    /// meshDesignVariablesToNodalFieldVector for an example.
    /// @sa meshDesignVariablesToNodalFieldVector
    ElementFieldVector meshDesignVariablesToElementFieldVector(
        const design_variables::MeshDesignVariables& aMeshDesignVariables) const;

    /// @brief Converts all design variable field values to a map from global ID to scalar design value.
    auto nodalFieldToNodalIDMap(NodalFieldVectorReference aNodalField) const
        -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>;

    /// @brief Converts all element field values to a map from global ID to field value.
    auto elementFieldToElementIDMap(ElementFieldVectorReference aElementField) const
        -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>;

    /// @brief Converts all design variables to a map from ID to design variable.
    auto meshDesignVariablesToIDMap(const design_variables::MeshDesignVariables& aMeshDesignVariables) const
        -> std::unordered_map<design_variables::ScalarFieldValue::IndexType, double>;
};

}  // namespace plato::mesh

#endif
