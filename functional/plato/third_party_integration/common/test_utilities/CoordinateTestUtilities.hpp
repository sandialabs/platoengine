#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_TEST_UTILITIES_COORDINATETESTUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_COMMON_TEST_UTILITIES_COORDINATETESTUTILITIES

#include "plato/test_utilities/TestContext.hpp"

namespace plato::third_party_integration::common::test_utilities
{

template <typename Container3>
void test_double_equality_of_components(const Container3& aResult,
                                        const Container3& aGold,
                                        const plato::test_utilities::TestContext& aTestContext)
{
    EXPECT_DOUBLE_EQ(aResult.mX, aGold.mX) << aTestContext;
    EXPECT_DOUBLE_EQ(aResult.mY, aGold.mY) << aTestContext;
    EXPECT_DOUBLE_EQ(aResult.mZ, aGold.mZ) << aTestContext;
}

template <typename Container3>
void test_near_equality_of_components(const Container3& aResult,
                                      const Container3& aGold,
                                      const double aTolerance,
                                      const plato::test_utilities::TestContext& aTestContext)
{
    EXPECT_NEAR(aResult.mX, aGold.mX, aTolerance) << aTestContext;
    EXPECT_NEAR(aResult.mY, aGold.mY, aTolerance) << aTestContext;
    EXPECT_NEAR(aResult.mZ, aGold.mZ, aTolerance) << aTestContext;
}

}  // namespace plato::third_party_integration::common::test_utilities

#endif
