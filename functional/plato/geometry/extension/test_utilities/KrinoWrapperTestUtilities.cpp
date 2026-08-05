#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"

#include <filesystem>
#include <set>
#include <string>

#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

namespace plato::geometry::extension::test_utilities
{
namespace
{
namespace tpik = third_party_integration::krino;
}

auto make_krino_wrapper_from_vector_values(const std::filesystem::path& aFileName,
                                           const InitialLevelSetValues& aInitialLevelSetValues,
                                           const third_party_integration::krino::VoidPhase& aVoidPhase) -> KrinoWrapper
{
    return KrinoWrapper{tpik::test_utilities::background_mesh_from_vector(aFileName, aInitialLevelSetValues.mValue),
                        aVoidPhase, tpik::SnappingParameters{}};
}

auto make_krino_wrapper_from_level_set_primitives(const std::filesystem::path& aFileName,
                                                  const tpik::LevelSetPrimitives& aLevelSetPrimitives,
                                                  const third_party_integration::krino::VoidPhase& aVoidPhase,
                                                  const std::set<std::string>& aFixedBlockNames,
                                                  const tpik::SnappingParameters& aSnappingParameters) -> KrinoWrapper
{
    return KrinoWrapper{tpik::background_mesh_from_primitives(aFileName, aLevelSetPrimitives, aFixedBlockNames),
                        aVoidPhase, aSnappingParameters};
}

}  // namespace plato::geometry::extension::test_utilities
