#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"

#include <vector>

#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

namespace plato::geometry::extension::test_utilities
{
namespace
{
namespace tpik = third_party_integration::krino;
}

auto make_krino_wrapper_from_vector_values(const std::filesystem::path& aFileName,
                                           const InitialLevelSetValues& aInitialLevelSetValues) -> KrinoWrapper
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aFileName);
    auto tLevelSet = tpik::test_utilities::make_level_set_field_from_vector(*tKrinoMesh, aInitialLevelSetValues.mValue);
    return KrinoWrapper{std::move(tKrinoMesh), std::move(tLevelSet), tpik::SnappingParameters{}};
}

auto make_krino_wrapper_from_level_set_primitives(const std::filesystem::path& aFileName,
                                                  const tpik::LevelSetPrimitives& aLevelSetPrimitives,
                                                  const std::set<std::string>& aFixedBlockNames) -> KrinoWrapper
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aFileName, aFixedBlockNames);
    auto tLevelSet = tpik::make_level_set_field_from_primitives(aLevelSetPrimitives, tKrinoMesh->bulk_data());
    return KrinoWrapper{std::move(tKrinoMesh), std::move(tLevelSet), tpik::SnappingParameters{}};
}

}  // namespace plato::geometry::extension::test_utilities
