#include <gtest/gtest.h>

#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"
namespace plato::geometry::extension::unittest
{
TEST(DensityTopologyValidation, ValidateMeshName)
{
    auto tDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();
    EXPECT_FALSE(library::detail::validate_mesh_name(tDensityTopology).has_value());
    tDensityTopology.mesh_name = boost::none;
    EXPECT_TRUE(library::detail::validate_mesh_name(tDensityTopology).has_value());
}

TEST(DensityTopologyValidation, ValidateOutputName)
{
    auto tDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();
    EXPECT_FALSE(detail::validate_output_name(tDensityTopology).has_value());
    tDensityTopology.output_name = boost::none;
    EXPECT_TRUE(detail::validate_output_name(tDensityTopology).has_value());
}

TEST(DensityTopologyValidation, ValidDensityTopologyInput)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();

    std::vector<std::string> tMessages;
    tMessages = library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_TRUE(tMessages.empty());
}

TEST(DensityTopologyValidation, InvalidDensityTopologyInput)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();
    tInput.mDensityTopology->filter_radius = 1.0;

    std::vector<std::string> tMessages;
    tMessages = library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1);
}
}  // namespace plato::geometry::extension::unittest