#include <gtest/gtest.h>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::geometry::library::unittest
{
namespace
{
std::optional<std::string> bogus_error(const input_parser::density_topology& aInput)
{
    if (aInput.mesh_name.value().mName == "trigger_bogus_test")
    {
        return "Bogus error for test_geometry_block";
    }
    else
    {
        return std::nullopt;
    }
}

[[maybe_unused]] static auto kDensityTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::density_topology>{[](const input_parser::density_topology& aInput)
                                                                 { return bogus_error(aInput); }};

// create some registered tests on geometry to make sure the right test is called for the right variant
[[maybe_unused]] static auto kBrickValidationRegistration =
    core::ValidationRegistration<input_parser::brick_shape_geometry>{[](const input_parser::brick_shape_geometry&)
                                                                     { return std::nullopt; }};
}  // namespace

TEST(GeometryValidation, InValidParsedInputNoGeometry)
{
    const auto tInput = input_parser::ParsedInput{};
    EXPECT_TRUE(plato::geometry::library::detail::validate_only_one_geometry(tInput).has_value());
}

TEST(GeometryValidation, ValidParsedInputOneGeometry)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mBrickShapeGeometry = input_parser::brick_shape_geometry{};
    EXPECT_FALSE(plato::geometry::library::detail::validate_only_one_geometry(tInput).has_value());
}

TEST(GeometryValidation, InValidParsedInputTwoGeometry)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mBrickShapeGeometry = input_parser::brick_shape_geometry{};
    tInput.mDensityTopology = input_parser::density_topology{};
    EXPECT_TRUE(plato::geometry::library::detail::validate_only_one_geometry(tInput).has_value());
}

TEST(GeometryValidation, MeshName)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mBrickShapeGeometry = plato::test_utilities::create_valid_brick_shape_geometry();

    std::vector<std::string> tMessages;
    tMessages = plato::geometry::library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 0u);
    tInput.mBrickShapeGeometry = input_parser::brick_shape_geometry{};
    tMessages = plato::geometry::library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(GeometryValidation, ValidInputCallsRightVariantTest)
{
    auto tInput = input_parser::ParsedInput{};
    auto tDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();
    tDensityTopology.mesh_name = input_parser::FileName{"trigger_bogus_test"};
    auto tBrickShapeGeometry = plato::test_utilities::create_valid_brick_shape_geometry();
    std::vector<std::string> tMessages;

    tInput.mBrickShapeGeometry = tBrickShapeGeometry;
    tMessages = plato::geometry::library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 0u);

    tInput.mBrickShapeGeometry = boost::none;
    tInput.mDensityTopology = tDensityTopology;
    tMessages = plato::geometry::library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1u);  // from bogus test geometry registration above

    tMessages.resize(0);
    tInput.mBrickShapeGeometry = tBrickShapeGeometry;  // now there are two geometries
    tMessages = plato::geometry::library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 2u);  // from bogus test geometry registration above and multiple geometries
}
}  // namespace plato::geometry::library::unittest