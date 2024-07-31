#ifndef PLATO_MESH_DESIGNVARIABLECONVERSION
#define PLATO_MESH_DESIGNVARIABLECONVERSION

#include <vector>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/utilities/NamedReference.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
using NodalDensityVector = utilities::NamedType<std::vector<double>, struct NodalDensityVectorTag>;
using ElementDensityVector = utilities::NamedType<std::vector<double>, struct ElementDensityVectorTag>;

using NodalDensityVectorReference =
    utilities::NamedReference<const std::vector<double>, struct NodalDensityVectorReferenceTag>;
using ElementDensityVectorReference =
    utilities::NamedReference<const std::vector<double>, struct ElementDensityVectorReferenceTag>;

struct DesignVariablesConversion : public Mesh
{
    explicit DesignVariablesConversion(Mesh aMesh);

    /// @brief Maps the nodal densities in @a aDensities to a MeshDesignVariables object using the node, element, and
    /// block info in the mesh.
    ///
    /// The ordering of @a aDensities is assumed to match that of the vector returned by
    /// meshDesignVariablesToNodalDensityVector.
    /// @pre The size of @a aDensities must be equal to the total number of nodes in the design domain of @a aMesh,
    /// which is given by numberOfDesignDomainElements in EntityCounts.
    MeshDesignVariables nodalDensitiesToMeshDesignVariables(NodalDensityVectorReference aDensities) const;

    /// @brief Maps the element densities in @a aDensities to a MeshDesignVariables object using the node, element, and
    /// block info in the mesh.
    /// @pre The size of @a aDensities must be equal to the total number of elements in the design domain of @a aMesh,
    /// which is given by numberOfDesignDomainElements in EntityCounts.
    MeshDesignVariables elementDensitiesToMeshDesignVariables(ElementDensityVectorReference aDensities) const;

    /// @brief Maps the nodal density field contained in @a aMeshDesignVariables to a vector.
    ///
    /// The ordering of the resulting vector will be sorted by global node ID. For example if @a aMeshDesignVariables
    /// has the data (with the form {block_id : [[global_id, density], [global_id, density]]}):
    /// @verbatim
    /// { 1 : [[1, 0], [4, 1], [5, 0.5]], 2 : [[2, 1], [3, 0], [6, 0.5]]}
    /// @endverbatim
    /// The result will be:
    /// @verbatim
    /// [0, 1, 0, 1, 0.5, 0.5]
    /// @endverbatim
    NodalDensityVector meshDesignVariablesToNodalDensityVector(const MeshDesignVariables& aMeshDesignVariables) const;

    /// @brief Maps the nodal density field contained in @a aMeshDesignVariables to a vector.
    ///
    /// The ordering of the resulting vector will be sorted by global element ID. See
    /// meshDesignVariablesToNodalDensityVector for an example.
    /// @sa meshDesignVariablesToNodalDensityVector
    ElementDensityVector meshDesignVariablesToElementDensityVector(
        const MeshDesignVariables& aMeshDesignVariables) const;
};

}  // namespace plato::mesh

#endif
