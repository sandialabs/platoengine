#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/Vector3Serialization.hpp"

namespace plato::third_party_integration::common::unittest
{

TEST(Vector3Serialization, Test)
{
    auto tStream = std::stringstream{};
    auto tSaveArchive = boost::archive::binary_oarchive{tStream};

    const auto tStoredVector = Vector3{.x = 1, .y = 2, .z = 3};
    tSaveArchive << tStoredVector;

    auto tLoadedVector = Vector3{};
    auto tLoadArchive = boost::archive::binary_iarchive{tStream};
    tLoadArchive >> tLoadedVector;

    EXPECT_EQ(tStoredVector, tLoadedVector);
}

}  // namespace plato::third_party_integration::common::unittest
