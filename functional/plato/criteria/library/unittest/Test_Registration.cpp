#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
[[nodiscard]] auto make_test_criterion_function() -> plato::criteria::library::CriterionFunction
{
    return core::make_function([](const core::MeshProxy&) { return 0.0; },
                               [](const core::MeshProxy&) {
                                   return linear_algebra::DynamicVector<double>{1.0, 2.0};
                               });
}

[[maybe_unused]] static auto kTestCriterionRegistration = plato::criteria::library::CriterionRegistration{
    "test", [](const plato::criteria::library::CriterionInput&) { return make_test_criterion_function(); }};

}  // namespace

TEST(CriterionRegistration, PhonyCriterion)
{
    EXPECT_TRUE(plato::criteria::library::is_criterion_function_registered("test"));
}

TEST(CriterionRegistration, NodalSum)
{
    const std::string_view tNodalSumName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kNodalSum).value();
    EXPECT_TRUE(plato::criteria::library::is_criterion_function_registered(tNodalSumName));
}

TEST(CriterionRegistration, CustomApp)
{
    const std::string_view tCustomAppName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kCustomApp).value();
    EXPECT_TRUE(plato::criteria::library::is_criterion_function_registered(tCustomAppName));
}
}  // namespace plato::criteria::library::unittest
