#ifndef PLATO_TEST_UTILITIES_CONTAINERS
#define PLATO_TEST_UTILITIES_CONTAINERS

#include <gtest/gtest.h>

#include <boost/range/combine.hpp>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{
/// @brief This function checks that each entry in @a aContainer1 and @a aContainer2 are within absolute tolerance of @a
/// aAbsoluteTolerance using the gtest test macro `EXPECT_NEAR`.
/// @note This checks the container sizes with `ASSERT_EQ`.
template <typename Container1, typename Container2>
void expect_container_entries_near(const Container1& aContainer1,
                                   const Container2& aContainer2,
                                   const double aAbsoluteTolerance,
                                   const TestContext& aTestContext)
{
    ASSERT_EQ(aContainer1.size(), aContainer2.size()) << aTestContext;
    for (const auto& [aEntry1, aEntry2] : boost::range::combine(aContainer1, aContainer2))
    {
        EXPECT_NEAR(aEntry1, aEntry2, aAbsoluteTolerance) << aTestContext;
    }
}
}  // namespace plato::test_utilities

#endif
