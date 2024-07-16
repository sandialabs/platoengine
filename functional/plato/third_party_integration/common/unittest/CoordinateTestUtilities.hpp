#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_UNITTEST_COORDINATETESTUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_COMMON_UNITTEST_COORDINATETESTUTILITIES

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common::unittest
{

template <typename Container3>
void test_double_equality_of_components(const Container3& aResult,
                                        const Container3& aGold,
                                        const test_utilities::TestContext& aTestContext)
{
    EXPECT_DOUBLE_EQ(aResult.x, aGold.x) << aTestContext;
    EXPECT_DOUBLE_EQ(aResult.y, aGold.y) << aTestContext;
    EXPECT_DOUBLE_EQ(aResult.z, aGold.z) << aTestContext;
}

}  // namespace plato::third_party_integration::common::unittest

#endif
