#ifndef PLATO_GEOMETRY_EXTENSION_TESTUTILITIES_KRINOWRAPPERTESTUTILITIES
#define PLATO_GEOMETRY_EXTENSION_TESTUTILITIES_KRINOWRAPPERTESTUTILITIES

#include <filesystem>
#include <optional>
#include <vector>

#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::geometry::extension::test_utilities
{
using InitialLevelSetValues = utilities::NamedType<std::vector<double>, struct InitialLevelSetValuesTag>;

/// @brief Helper function to facilitate making a KrinoWrapper.
///
/// Read a mesh from @a aFileName, and use the level set values specified in @a aInitialLevelSetValues to assign the
/// level set field.
/// @pre the size of @a aInitialLevelSetValues must equal the total number of background nodes in the mesh.
[[nodiscard]] auto make_krino_wrapper_from_vector_values(
    const std::filesystem::path& aFileName, const InitialLevelSetValues& aInitialLevelSetValues) -> KrinoWrapper;

/// @brief Helper function to facilitate making a KrinoWrapper.
///
/// Read a mesh from @a aFileName, and use the level set primitives @a aLevelSetPrimitives. This is primarily used to
/// specify a starting point for a level set optimization.
[[nodiscard]] auto make_krino_wrapper_from_level_set_primitives(
    const std::filesystem::path& aFileName,
    const third_party_integration::krino::LevelSetPrimitives& aLevelSetPrimitives,
    const std::set<std::string>& aFixedBlockNames = {}) -> KrinoWrapper;

}  // namespace plato::geometry::extension::test_utilities

#endif
