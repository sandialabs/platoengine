#include "plato/filter/extension/kernel_filters/test_utilities/ReflectionMeshTestUtility.hpp"

#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::filter::extension::kernel_filters::test_utilities
{
void create_mesh_for_reflection_filter(const std::filesystem::path& aMeshFile,
                                       const third_party_integration::stk_io::CommandBounds& aLowerBounds)
{
    namespace tpis = third_party_integration::stk_io;
    const auto tCommandGenerator = tpis::CommandGenerator{.mElements = {10, 10, 10},
                                                          .mLowerBounds = aLowerBounds,
                                                          .mUpperBounds = {1, 1, 1},
                                                          .mType = tpis::CommandElementType::Hex};
    tpis::write_mesh(aMeshFile, tCommandGenerator.toString());
}
}  // namespace plato::filter::extension::kernel_filters::test_utilities
