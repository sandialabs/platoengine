#include <gtest/gtest.h>

#include <boost/fusion/adapted/struct/define_struct.hpp>

#include "plato/geometry/library/GeometryInputBuilder.hpp"
// clang-format off
BOOST_FUSION_DEFINE_STRUCT((plato)(geometry)(library)(detail),
                           TestStructNoGeometryTypes,
                           (int, mInt)
                           (double, mDouble))

BOOST_FUSION_DEFINE_STRUCT((plato)(geometry)(library)(detail),
                           TestStructWithGeometryTypes,
                           (plato::input_parser::density_topology, mDensityTopology1)
                           (double, mDouble)
                           (plato::input_parser::brick_shape_geometry, mBrickShapeGeometry)
                           (boost::optional<plato::input_parser::density_topology>, mDensityTopology2))
// clang-format on
namespace plato::geometry::library::unittest
{
TEST(GeometryInputBuilder, TupleIfGeometryInput)
{
    namespace pfgld = plato::geometry::library::detail;
    {
        [[maybe_unused]] const std::tuple<> tEmptyTuple = pfgld::tuple_if_geometry_input<int>();
        EXPECT_EQ(std::tuple_size_v<decltype(tEmptyTuple)>, 0);
    }  // namespace plato::geometry::library::detail;
    {
        [[maybe_unused]] const std::tuple<input_parser::density_topology> tTuple =
            pfgld::tuple_if_geometry_input<input_parser::density_topology>();
        EXPECT_EQ(std::tuple_size_v<decltype(tTuple)>, 1);
    }
}

TEST(GeometryInputBuilder, CatIfIsGeometryInput)
{
    namespace pfgld = plato::geometry::library::detail;
    {
        auto tInitial = std::tuple<>{};
        constexpr std::size_t tNumFields = boost::fusion::result_of::size<detail::TestStructNoGeometryTypes>::value;
        [[maybe_unused]] const std::tuple<> tEmptyTuple =
            pfgld::cat_if_is_geometry_input<detail::TestStructNoGeometryTypes, tNumFields - 1>(tInitial);
        EXPECT_EQ(std::tuple_size_v<decltype(tEmptyTuple)>, 0);
    }  // namespace plato::geometry::library::detail;
    {
        auto tInitial = std::tuple<>{};
        constexpr std::size_t tNumFields = boost::fusion::result_of::size<detail::TestStructWithGeometryTypes>::value;
        [[maybe_unused]] const auto tResultTuple =
            pfgld::cat_if_is_geometry_input<detail::TestStructWithGeometryTypes, tNumFields - 1>(tInitial);
        EXPECT_EQ(std::tuple_size_v<decltype(tResultTuple)>, 3);
    }
}

TEST(GeometryInputBuilder, MakeGeometryInputTuple)
{
    namespace pfgld = plato::geometry::library::detail;
    {
        [[maybe_unused]] const auto tResult = pfgld::make_geometry_input_tuple{}(detail::TestStructNoGeometryTypes{});
        EXPECT_EQ(std::tuple_size_v<decltype(tResult)>, 0);
    }  // namespace plato::geometry::library::detail;
    {
        [[maybe_unused]] const auto tResult = pfgld::make_geometry_input_tuple{}(detail::TestStructWithGeometryTypes{});
        EXPECT_EQ(std::tuple_size_v<decltype(tResult)>, 3);
        using ResultType = decltype(tResult);
        static_assert(
            std::is_same_v<std::decay_t<std::tuple_element_t<0, ResultType>>, input_parser::density_topology>);
        static_assert(
            std::is_same_v<std::decay_t<std::tuple_element_t<1, ResultType>>, input_parser::brick_shape_geometry>);
        static_assert(
            std::is_same_v<std::decay_t<std::tuple_element_t<2, ResultType>>, input_parser::density_topology>);
    }
}

TEST(GeometryInputBuilder, VariantFromTuple)
{
    namespace pfgld = plato::geometry::library::detail;
    using TupleType = std::tuple<int, double, char>;
    using VariantType = typename pfgld::VariantFromTuple<TupleType>::type;
    static_assert(std::is_same_v<VariantType, std::variant<int, double, char>>);
}

TEST(GeometryInputBuilder, GeometryInputVariant)
{
    namespace pfgld = plato::geometry::library::detail;
    using TestInput = pfgld::GeometryInputVariant<pfgld::TestStructWithGeometryTypes>;
    using ExpectedType = std::variant<input_parser::density_topology, input_parser::brick_shape_geometry,
                                      input_parser::density_topology>;
    static_assert(std::is_same_v<TestInput, ExpectedType>);
}

TEST(GeometryInputBuilder, GeometryInput)
{
    namespace pfgld = plato::geometry::library::detail;
    using TestInput = pfgld::GeometryInputVariant<input_parser::ParsedInput>;
    static_assert(std::variant_size_v<TestInput> == 2);
    static_assert(std::is_same_v<std::variant_alternative_t<0, TestInput>, input_parser::density_topology>);
    static_assert(std::is_same_v<std::variant_alternative_t<1, TestInput>, input_parser::brick_shape_geometry>);
}

TEST(GeometryInputBuilder, ValidatedGeometryInput)
{
    namespace pfgld = plato::geometry::library::detail;
    using TestInput = pfgld::ValidatedGeometryInputVariant<input_parser::ParsedInput>;
    static_assert(std::variant_size_v<TestInput> == 2);
    static_assert(std::is_same_v<std::variant_alternative_t<0, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::density_topology>>);
    static_assert(std::is_same_v<std::variant_alternative_t<1, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::brick_shape_geometry>>);
}
}  // namespace plato::geometry::library::unittest