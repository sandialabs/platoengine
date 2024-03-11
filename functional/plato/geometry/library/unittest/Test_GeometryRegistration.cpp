#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
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
    return core::make_function([](const linear_algebra::DynamicVector<double>&) { return core::MeshProxy{}; },
                               [](const linear_algebra::DynamicVector<double>&)
                               { return linear_algebra::JacobianMultiplier{}; });
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

TEST(GeometryRegistrationUtilities, GeometryInputAllEmpty)
{
    EXPECT_THROW(auto tGeometryInput = first_geometry_input(input_parser::ParsedInput{}),
                 plato::utilities::Exception);
}
}  // namespace plato::geometry::library::unittest