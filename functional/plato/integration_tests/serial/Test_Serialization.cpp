#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "plato/test_utilities/TwoDTestTypesSerialization.hpp"

namespace plato::integration_tests::serial
{
TEST(Serialization, TwoDTestTypes)
{
    auto tStream = std::stringstream{};
    auto tSaveArchive = boost::archive::binary_oarchive{tStream};

    const auto tStoredVector = test_utilities::TwoDVector{1.0, 2.0};
    tSaveArchive << tStoredVector;

    auto tLoadedVector = test_utilities::TwoDVector{0.0, 0.0};
    auto tLoadArchive = boost::archive::binary_iarchive{tStream};
    tLoadArchive >> tLoadedVector;
    EXPECT_EQ(tLoadedVector, tStoredVector);
}
}  // namespace plato::integration_tests::serial
