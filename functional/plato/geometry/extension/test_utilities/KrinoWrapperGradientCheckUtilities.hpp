#ifndef PLATO_GEOMETRY_EXTENSION_TESTUTILITIES_KRINOWRAPPERGRADIENTCHECKUTILITIES
#define PLATO_GEOMETRY_EXTENSION_TESTUTILITIES_KRINOWRAPPERGRADIENTCHECKUTILITIES

#include <filesystem>
#include <vector>

namespace plato::geometry::extension::test_utilities
{

/// @brief Use the mesh @a aMeshToLoad and the perturbed level set field @a aPerturbedLevelSetField to generate a krino
/// wrapper. Then sum up the nodal coordinates of the cut mesh.
[[nodiscard]] auto accumulate_cut_node_coordinates(const std::filesystem::path& aMeshToLoad,
                                                   const std::vector<double>& aPerturbedLevelSetField) -> double;

}  // namespace plato::geometry::extension::test_utilities

#endif
