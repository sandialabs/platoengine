#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Function.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::filter::library::unittest
{
namespace
{
[[nodiscard]] auto make_test_filter_function() -> FilterFunction
{
    return FilterFunction([](const analysis::AnalysisDomainMesh&) { return analysis::AnalysisDomainMesh{}; },
                          [](const analysis::AnalysisDomainMesh&) { return FilterJacobian{}; },
                          [](const analysis::AnalysisDomainMesh&) { return FilterAdjointJacobian{FilterJacobian{}}; });
}

[[maybe_unused]] static auto kTestFilterRegistration =
    NewFilterRegistration{"test", [](const NewValidatedFilterInput&) { return make_test_filter_function(); }};
}  // namespace

TEST(FilterRegistration, PhonyFilter) { EXPECT_TRUE(is_new_filter_function_registered("test")); }

}  // namespace plato::filter::library::unittest
