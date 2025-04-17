#include <gtest/gtest.h>

#include <filesystem>

#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
constexpr std::string_view kMeshFile = "test.exo";
}  // namespace

TEST(FilterMeshUtilities, GetFilterRadius)
{
    constexpr double tElementLength = 2;
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 1},
        {-tElementLength / 2, -tElementLength / 2, -tElementLength / 2},
        {tElementLength / 2, tElementLength / 2, tElementLength / 2},
        third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    auto tFilter = create_valid_helmholtz_filter_input();
    constexpr double tFilterRadius = 21;
    tFilter.filter_radius = tFilterRadius;

    EXPECT_EQ(detail::get_filter_radius(tFilter, kMeshFile), tFilterRadius);  // default is absolute

    tFilter.use_relative_radius = false;
    EXPECT_EQ(detail::get_filter_radius(tFilter, kMeshFile), tFilterRadius);

    tFilter.use_relative_radius = true;
    EXPECT_EQ(detail::get_filter_radius(tFilter, kMeshFile), tElementLength * tFilterRadius);

    std::filesystem::remove(kMeshFile);
}
}  // namespace plato::filter::extension::unittest
