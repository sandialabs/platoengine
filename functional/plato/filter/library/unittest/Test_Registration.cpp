#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

namespace input_parser
{
struct density_topology;
}

namespace plato::filter::library::unittest
{
namespace
{
[[nodiscard]] auto make_test_filter_function() -> FilterFunction
{
    return core::make_function([](const core::MeshProxy&) { return core::MeshProxy{}; },
                               [](const core::MeshProxy&) { return FilterJacobian{}; });
}

[[maybe_unused]] static auto kTestFilterRegistration =
    FilterRegistration{"test", [](const input_parser::density_topology&) { return make_test_filter_function(); }};
}  // namespace

TEST(FilterRegistration, PhonyFilter) { EXPECT_TRUE(is_filter_function_registered("test")); }

TEST(FilterRegistration, Identity) { EXPECT_TRUE(is_filter_function_registered("identity")); }
}  // namespace plato::filter::library::unittest
