#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"

#include <filesystem>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "plato/geometry/extension/KrinoWrapper.hpp"
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
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aFileName);
    auto tLevelSet = tpik::test_utilities::make_level_set_field_from_vector(*tKrinoMesh, aInitialLevelSetValues.mValue);
    return KrinoWrapper{std::move(tKrinoMesh), std::move(tLevelSet), aVoidPhase, tpik::SnappingParameters{}};
}

auto make_krino_wrapper_from_level_set_primitives(const std::filesystem::path& aFileName,
                                                  const tpik::LevelSetPrimitives& aLevelSetPrimitives,
                                                  const third_party_integration::krino::VoidPhase& aVoidPhase,
                                                  const std::set<std::string>& aFixedBlockNames,
                                                  const tpik::SnappingParameters& aSnappingParameters) -> KrinoWrapper
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aFileName, aFixedBlockNames);
    auto tLevelSet = tpik::make_level_set_field_from_primitives(aLevelSetPrimitives, tKrinoMesh->bulk_data());
    return KrinoWrapper{std::move(tKrinoMesh), std::move(tLevelSet), aVoidPhase, aSnappingParameters};
}

}  // namespace plato::geometry::extension::test_utilities
