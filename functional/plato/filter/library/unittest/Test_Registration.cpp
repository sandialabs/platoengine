#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

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
    FilterRegistration{"test", [](const ValidatedFilterInput&) { return make_test_filter_function(); }};
}  // namespace

TEST(FilterRegistration, PhonyFilter) { EXPECT_TRUE(is_filter_function_registered("test")); }

TEST(FilterRegistration, Identity) { EXPECT_TRUE(is_filter_function_registered("identity_filter")); }

TEST(FilterRegistration, Helmholtz) { EXPECT_TRUE(is_filter_function_registered("helmholtz_filter")); }

TEST(FilterRegistration, Kernel) { EXPECT_TRUE(is_filter_function_registered("kernel_filter")); }

TEST(FilterRegistration, FilterInput)
{
    using TestInput = FilterInput;
    static_assert(std::variant_size_v<TestInput> == 3);
    static_assert(std::is_same_v<std::variant_alternative_t<0, TestInput>, input_parser::kernel_filter>);
    static_assert(std::is_same_v<std::variant_alternative_t<1, TestInput>, input_parser::helmholtz_filter>);
    static_assert(std::is_same_v<std::variant_alternative_t<2, TestInput>, input_parser::identity_filter>);
}

TEST(FilterRegistration, ValidatedFilterInput)
{
    using TestInput = ValidatedFilterInput::RawInputType;
    static_assert(std::variant_size_v<TestInput> == 3);
    static_assert(std::is_same_v<std::variant_alternative_t<0, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::kernel_filter>>);
    static_assert(std::is_same_v<std::variant_alternative_t<1, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::helmholtz_filter>>);
    static_assert(std::is_same_v<std::variant_alternative_t<2, TestInput>,
                                 core::ValidatedInputTypeWrapper<input_parser::identity_filter>>);
}
}  // namespace plato::filter::library::unittest
