#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"

#include <vector>

#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
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
    auto tBackgroundMesh = tpik::initialize_background_mesh(aFileName);
    tpik::test_utilities::populate_level_set_field_from_vector(tBackgroundMesh, aInitialLevelSetValues.mValue);
    return KrinoWrapper{std::move(tBackgroundMesh), aVoidPhase, tpik::SnappingParameters{}};
}

auto make_krino_wrapper_from_level_set_primitives(const std::filesystem::path& aFileName,
                                                  const tpik::LevelSetPrimitives& aLevelSetPrimitives,
                                                  const third_party_integration::krino::VoidPhase& aVoidPhase,
                                                  const std::set<std::string>& aFixedBlockNames) -> KrinoWrapper
{
    auto tBackgroundMesh = tpik::initialize_background_mesh(aFileName, aFixedBlockNames);
    tpik::populate_level_set_field_from_primitives(aLevelSetPrimitives, tBackgroundMesh);
    return KrinoWrapper{std::move(tBackgroundMesh), aVoidPhase, tpik::SnappingParameters{}};
}

}  // namespace plato::geometry::extension::test_utilities
