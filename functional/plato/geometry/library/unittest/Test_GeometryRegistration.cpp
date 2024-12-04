#include <gtest/gtest.h>

#include <variant>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::library::unittest
{
namespace
{
[[nodiscard]] auto make_test_geometry_function() -> FactoryTypes::Compute
{
    return FactoryTypes::Compute{
        [](const linear_algebra::DynamicVector<double>&) { return analysis::AnalysisDomainMesh{}; },
        [](const linear_algebra::DynamicVector<double>&) { return linear_algebra::JacobianMultiplier{}; },
        [](const linear_algebra::DynamicVector<double>&)
        { return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{}}; }};
}

[[maybe_unused]] static auto kTestGeometryRegistration =
    GeometryRegistration{"test", [](const ValidatedGeometryInput&)
                         {
                             return FactoryTypes{make_test_geometry_function(), linear_algebra::DynamicVector<double>{},
                                                 std::make_pair(std::vector<double>{}, std::vector<double>{}),
                                                 std::function<void(const linear_algebra::DynamicVector<double>&)>{}};
                         }};
}  // namespace

TEST(GeometryRegistration, PhonyGeometry) { EXPECT_TRUE(is_geometry_function_registered("test")); }

TEST(GeometryRegistration, BrickGeometry) { EXPECT_TRUE(is_geometry_function_registered("brick_shape_geometry")); }

TEST(GeometryRegistration, DensityTopology) { EXPECT_TRUE(is_geometry_function_registered("density_topology")); }

TEST(GeometryRegistration, GeometryInputAllEmpty)
{
    EXPECT_THROW(auto tGeometryInput = first_geometry_input(input_parser::ParsedInput{}), plato::utilities::Exception);
}

TEST(GeometryRegistration, GeometryInput)
{
    using TestInput = GeometryInput;
    static_assert(std::variant_size_v<TestInput> == 3);
    static_assert(std::is_same_v<std::variant_alternative_t<0, TestInput>, input_parser::level_set_topology>);
    static_assert(std::is_same_v<std::variant_alternative_t<1, TestInput>, input_parser::density_topology>);
    static_assert(std::is_same_v<std::variant_alternative_t<2, TestInput>, input_parser::brick_shape_geometry>);
}

TEST(GeometryRegistration, ValidatedGeometryInput)
{
    using TestInput = ValidatedGeometryInput::RawInputType;
    static_assert(std::variant_size_v<TestInput> == 3);
    static_assert(std::is_same_v<std::variant_alternative_t<0, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::level_set_topology>>);
    static_assert(std::is_same_v<std::variant_alternative_t<1, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::density_topology>>);
    static_assert(std::is_same_v<std::variant_alternative_t<2, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::brick_shape_geometry>>);
}

}  // namespace plato::geometry::library::unittest
