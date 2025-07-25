#include <gtest/gtest.h>

#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>

#include "plato//test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/FilterConjunction.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestUtilities.hpp"

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
constexpr auto kTestAttribute1Name = std::string_view{"test attribute 1"};
constexpr auto kTestAttribute2Name = std::string_view{"test attribute 2"};

constexpr auto kAttributeValue1 = 42;
constexpr auto kAttributeValue2 = 84;
}  // namespace
}  // namespace plato::third_party_integration::boost_log::unittest

BOOST_LOG_ATTRIBUTE_KEYWORD(test_attribute_1,
                            plato::third_party_integration::boost_log::unittest::kTestAttribute1Name.data(),
                            int)
BOOST_LOG_ATTRIBUTE_KEYWORD(test_attribute_2,
                            plato::third_party_integration::boost_log::unittest::kTestAttribute2Name.data(),
                            int)

namespace plato::third_party_integration::boost_log::unittest
{
namespace
{
[[nodiscard]] auto filter_for_attribute(const auto& test_attribute_keyword, const int aAttribute) -> boost::log::filter
{
    return boost::log::filter{boost::log::expressions::has_attr(test_attribute_keyword) &&
                              test_attribute_keyword == aAttribute};
}

void check_attribute_set(const int aValue1,
                         const int aValue2,
                         const bool aExpected,
                         const plato::test_utilities::TestContext& aTestContext)
{
    auto tTestAttributes = test_utilities::attribute_set(kTestAttribute1Name, aValue1);
    tTestAttributes.insert(boost::log::attribute_name{std::string{kTestAttribute2Name}},
                           boost::log::attributes::constant<int>(aValue2));

    const auto tTestAttributeSet = test_utilities::attribute_value_set(tTestAttributes);

    const auto tConjoinedFilter = filter_conjunction(filter_for_attribute(test_attribute_1, kAttributeValue1),
                                                     filter_for_attribute(test_attribute_2, kAttributeValue2));

    EXPECT_EQ(tConjoinedFilter(tTestAttributeSet), aExpected) << aTestContext;
}
}  // namespace

TEST(FilterConjunction, FilterConjunction)
{
    {
        const auto tExpectTrue = true;
        check_attribute_set(kAttributeValue1, kAttributeValue2, tExpectTrue, TEST_CONTEXT("Filter matches"));
    }
    {
        const auto tExpectFalse = false;
        const auto tNonMatchingAttribute1 = 2 * kAttributeValue1;
        check_attribute_set(tNonMatchingAttribute1, kAttributeValue2, tExpectFalse,
                            TEST_CONTEXT("Attribute 1 does not match"));
    }
    {
        const auto tExpectFalse = false;
        const auto tNonMatchingAttribute2 = 2 * kAttributeValue2;
        check_attribute_set(kAttributeValue1, tNonMatchingAttribute2, tExpectFalse,
                            TEST_CONTEXT("Attribute 2 does not match"));
    }
    {
        const auto tExpectFalse = false;
        const auto tAttribute1 = 2 * kAttributeValue1;
        const auto tAttribute2 = 2 * kAttributeValue2;
        check_attribute_set(tAttribute1, tAttribute2, tExpectFalse, TEST_CONTEXT("Neither attributes match"));
    }
}
}  // namespace plato::third_party_integration::boost_log::unittest
