#ifndef PLATO_GEOMETRY_EXTENSION_DENSITYRESULTTOCUTMESH
#define PLATO_GEOMETRY_EXTENSION_DENSITYRESULTTOCUTMESH

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"

namespace plato::geometry::extension
{

/// @brief Take an analysis domain mesh @a aAnalysisDomainMesh from a density to result and the information in the input
/// @a aInput to create a cut mesh at the density contour specified by 'cut_mesh_density_threshold'.
void write_density_result_as_cut_mesh(analysis::AnalysisDomainMesh aAnalysisDomainMesh,
                                      const input_parser::density_topology& aInput,
                                      const boost::mpi::communicator& aCommunicator);

namespace detail
{

/// @brief Take an element centered AnalysisDomainMesh @a aElementAnalysisDomainMesh and using the mesh @a aMesh convert
/// to a nodal centered AnalysisDomainMesh
[[nodiscard]] auto convert_to_nodal(const analysis::AnalysisDomainMesh& aElementAnalysisDomainMesh,
                                    const mesh::Mesh& aMesh) -> analysis::AnalysisDomainMesh;

/// @brief Take an analysis domain mesh @a aAnalysisDomainMesh from a density result that goes from 0 -> 1, and subtract
/// a density threshold @a aDensityThreshold
[[nodiscard]] auto transform_to_level_set_field(analysis::AnalysisDomainMesh aAnalysisDomainMesh,
                                                double aDensityThreshold) -> analysis::AnalysisDomainMesh;

/// @brief Take a vector of doubles and check the bounds to determine if a zero crossing exists
[[nodiscard]] auto zero_crossing_exists(const std::vector<double>& aVector) -> bool;

}  // namespace detail

}  // namespace plato::geometry::extension

#endif
