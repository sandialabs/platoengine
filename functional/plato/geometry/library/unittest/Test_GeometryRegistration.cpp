#include <gtest/gtest.h>

#include <variant>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
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

[[maybe_unused]] static auto kTestGeometryRegistration = NewGeometryRegistration{
    "test", [](const NewValidatedGeometryInput&)
    {
        return FactoryTypes{make_test_geometry_function(), linear_algebra::DynamicVector<double>{},
                            std::make_pair(std::vector<double>{}, std::vector<double>{}),
                            std::function<void(const linear_algebra::DynamicVector<double>&, const OutputInfo&)>{}};
    }};
}  // namespace

TEST(GeometryRegistration, PhonyGeometry) { EXPECT_TRUE(is_new_geometry_function_registered("test")); }

}  // namespace plato::geometry::library::unittest
