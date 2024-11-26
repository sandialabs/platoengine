#ifndef PLATO_GEOMETRY_LIBRARY_DESIGNVARIABLEADAPTER
#define PLATO_GEOMETRY_LIBRARY_DESIGNVARIABLEADAPTER

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::geometry::library
{
using DesignVariableToAnalysisMeshAdapterFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<analysis::AnalysisDomainMesh, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

using AnalysisMeshToDesignVariablesAdapterFunction =
    core::Function<const analysis::AnalysisDomainMesh&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

/// @brief Creates a Function that maps a vector of design variables to an AnalysisDomainMesh based on @a aMesh.
auto design_variables_to_analysis_mesh_adapter(const mesh::Mesh& aMesh) -> DesignVariableToAnalysisMeshAdapterFunction;

/// @brief Creates a Function that maps an AnalysisDomainMesh to a vector of design variables based on @a aMesh.
auto analysis_mesh_to_design_variables_adapter(const mesh::Mesh& aMesh) -> AnalysisMeshToDesignVariablesAdapterFunction;

}  // namespace plato::geometry::library

#endif
