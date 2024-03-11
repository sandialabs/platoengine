#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryRegistrationUtilities.hpp"

namespace plato::geometry::library::unittest
{
TEST(GeometryRegistrationUtilities, IsVariant)
{
    namespace pfgld = plato::geometry::library::detail;

    using TestVariant = std::variant<double, char, bool>;
    constexpr bool tDoubleInVariant = pfgld::kIsVariantMember<double, TestVariant>;
    EXPECT_TRUE(tDoubleInVariant);

    constexpr bool tIntInVariant = pfgld::kIsVariantMember<int, TestVariant>;
    EXPECT_FALSE(tIntInVariant);
}

TEST(GeometryRegistrationUtilities, MakeVariantWrongType)
{
    namespace pfgl = plato::geometry::library;
    namespace pfgld = plato::geometry::library::detail;
    const std::optional<pfgl::GeometryInput> tResult = pfgld::make_variant_if_geometry(42);
    EXPECT_FALSE(tResult.has_value());
}

TEST(GeometryRegistrationUtilities, MakeVariantRightTypeEmpty)
{
    namespace pfgl = plato::geometry::library;
    namespace pfgld = plato::geometry::library::detail;
    using ValidOptional = boost::optional<input_parser::density_topology>;
    const std::optional<pfgl::GeometryInput> tResult = pfgld::make_variant_if_geometry(ValidOptional{boost::none});
    EXPECT_FALSE(tResult.has_value());
}

TEST(GeometryRegistrationUtilities, MakeVariantRightTypeNotEmpty)
{
    namespace pfgl = plato::geometry::library;
    namespace pfgld = plato::geometry::library::detail;
    using ValidOptional = boost::optional<input_parser::density_topology>;
    const std::optional<pfgl::GeometryInput> tResult =
        pfgld::make_variant_if_geometry(ValidOptional{input_parser::density_topology{}});
    EXPECT_TRUE(tResult.has_value());
}

TEST(GeometryRegistrationUtilities, GeometryBlockAllEmpty)
{
    namespace pfgl = plato::geometry::library;
    namespace pfgld = plato::geometry::library::detail;
    const std::optional<pfgl::GeometryInput> tGeometryInput = pfgld::first_geometry_block(input_parser::ParsedInput{});
    EXPECT_FALSE(tGeometryInput.has_value());
}

TEST(GeometryRegistrationUtilities, GeometryBlockHasDensityTopology)
{
    namespace pfgl = plato::geometry::library;
    namespace pfgld = plato::geometry::library::detail;
    auto tInput = input_parser::ParsedInput{};
    tInput.mDensityTopology.emplace();
    const std::optional<pfgl::GeometryInput> tGeometryInput = pfgld::first_geometry_block(tInput);
    EXPECT_TRUE(tGeometryInput.has_value());
}

TEST(GeometryRegistrationUtilities, GeometryInputHasDensityTopology)
{
    namespace pfgl = plato::geometry::library;
    namespace pfgld = plato::geometry::library::detail;
    auto tInput = input_parser::ParsedInput{};
    tInput.mDensityTopology.emplace();
    EXPECT_NO_THROW(auto tGeometryInput = pfgld::first_geometry_block(tInput));
}

TEST(GeometryRegistrationUtilities, BlockName)
{
    namespace pfgl = plato::geometry::library;
    {
        const auto tGeometryInput = pfgl::GeometryInput{input_parser::density_topology{}};
        EXPECT_EQ(pfgl::detail::block_name(tGeometryInput), "density_topology");
    }  // namespace plato::geometry::library;
    {
        const auto tGeometryInput = pfgl::GeometryInput{input_parser::brick_shape_geometry{}};
        EXPECT_EQ(pfgl::detail::block_name(tGeometryInput), "brick_shape_geometry");
    }
}

TEST(GeometryRegistrationUtilities, GeometryBlocksVector)
{
    namespace pfgl = plato::geometry::library;
    {
        input_parser::ParsedInput tInput;
        EXPECT_EQ(pfgl::detail::geometry_blocks(tInput).size(), 0u);
    }  // namespace plato::geometry::library;
    {
        input_parser::ParsedInput tInput;
        tInput.mDensityTopology = input_parser::density_topology{};
        EXPECT_EQ(pfgl::detail::geometry_blocks(tInput).size(), 1u);
    }
    {
        input_parser::ParsedInput tInput;
        tInput.mDensityTopology = input_parser::density_topology{};
        tInput.mBrickShapeGeometry = input_parser::brick_shape_geometry{};
        EXPECT_EQ(pfgl::detail::geometry_blocks(tInput).size(), 2u);
    }
}
}  // namespace plato::geometry::library::unittest