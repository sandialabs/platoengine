#ifndef PLATO_GEOMETRY_EXTENSION_TESTUTILITIES_KRINOWRAPPERGRADIENTCHECKUTILITIES
#define PLATO_GEOMETRY_EXTENSION_TESTUTILITIES_KRINOWRAPPERGRADIENTCHECKUTILITIES

#include <filesystem>
#include <vector>

namespace plato::geometry::extension::test_utilities
{

[[nodiscard]] auto accumulate_cut_node_coordinates(const std::filesystem::path& aMeshToLoad,
                                                   const std::vector<double>& aPerturbedLevelSetField) -> double;

}

#endif
