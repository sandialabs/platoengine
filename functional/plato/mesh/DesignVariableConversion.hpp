#ifndef PLATO_MESH_DESIGNVARIABLECONVERSION
#define PLATO_MESH_DESIGNVARIABLECONVERSION

#include <vector>

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::mesh
{
/// @brief Maps the nodal densities in @a aDensities to a MeshDesignVariables object using the node, element, and block
/// info in @a aMesh.
///
/// @pre The size of @a aDensities must be equal to the total number of nodes in @a aMesh
/// @post The ordering of the densities in the returned MeshDesignVariables will be such that iterating over the
/// MeshDesignVariables with MeshDesignVariablesDensitiesView will match the order of @a aDensities. In other words, the
/// following assertions are true:
/// @code{.cpp}
/// assert(MeshDesignVariablesDensitiesView{aMesh}.size(), aDensities.size());
/// for(const auto& [aMeshDensity, aVectorDensityValue] : Zip{MeshDesignVariablesDensitiesView{aMesh}, aDensities})
///   assert(aMeshDensity.mDensity == aVectorDensityValue);
/// @endcode
MeshDesignVariables nodal_densities_to_mesh_design_variables(const std::vector<double>& aDensities, const Mesh& aMesh);

/// @brief Maps the element densities in @a aDensities to a MeshDesignVariables object using the node, element, and
/// block info in
/// @a aMesh.
///
/// @pre The size of @a aDensities must be equal to the total number of elements in @a aMesh
/// @post The ordering of the densities in the returned MeshDesignVariables will be such that iterating over the
/// MeshDesignVariables with MeshDesignVariablesDensitiesView will match the order of @a aDensities. In other words, the
/// following assertions are true:
/// @code{.cpp}
/// assert(MeshDesignVariablesDensitiesView{aMesh}.size(), aDensities.size());
/// for(const auto& [aMeshDensity, aVectorDensityValue] : Zip{MeshDesignVariablesDensitiesView{aMesh}, aDensities})
///   assert(aMeshDensity.mDensity == aVectorDensityValue);
/// @endcode
MeshDesignVariables element_densities_to_mesh_design_variables(const std::vector<double>& aDensities,
                                                               const Mesh& aMesh);

}  // namespace plato::mesh

#endif
