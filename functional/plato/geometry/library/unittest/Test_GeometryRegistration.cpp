#include <gtest/gtest.h>

#include <functional>
#include <utility>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/output/OutputInfo.hpp"

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

[[maybe_unused]] const auto kTestGeometryRegistration = GeometryRegistration{
    "test", [](const ValidatedGeometryInput&)
    {
        return FactoryTypes{
            make_test_geometry_function(), linear_algebra::DynamicVector<double>{},
            std::make_pair(std::vector<double>{}, std::vector<double>{}),
            std::function<void(const linear_algebra::DynamicVector<double>&, const output::OutputInfo&)>{}};
    }};
}  // namespace

TEST(GeometryRegistration, PhonyGeometry) { EXPECT_TRUE(is_geometry_function_registered("test")); }

}  // namespace plato::geometry::library::unittest
