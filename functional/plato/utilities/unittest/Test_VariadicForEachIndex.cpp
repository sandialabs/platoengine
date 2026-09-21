#include <gtest/gtest.h>

#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "plato/utilities/VariadicForEachIndex.hpp"

// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
    (plato)(utilities)(unittest),
    BoostFusionTestStruct, //NOLINT
    (int, mInteger)
    (double, mDouble)
    (std::string, mString) //NOLINT
)
// clang-format on

namespace plato::utilities::unittest
{

TEST(VariadicForEachIndex, AccumulateIndices)
{
    auto tIndexCounter = 0U;
    constexpr auto tNumberOfIndices = 4U;
    for_each_index<tNumberOfIndices>([&tIndexCounter]<std::size_t kIndex>() { tIndexCounter += kIndex; });
    EXPECT_EQ(tIndexCounter, tNumberOfIndices * (tNumberOfIndices - 1) / 2);
}

namespace
{
struct CollectBoostFusionMemberNames
{
    std::reference_wrapper<std::vector<std::string>> mMemberNames;
    template <std::size_t kIndex>
    void operator()() const
    {
        mMemberNames.get().emplace_back(
            boost::fusion::extension::struct_member_name<BoostFusionTestStruct, kIndex>::call());
    }
};

}  // namespace

TEST(VariadicForEachIndex, CollectBoostFusionStructMemberNames)
{
    std::vector<std::string> tMemberNames;
    constexpr auto tNumberOfMembers = boost::fusion::result_of::size<BoostFusionTestStruct>::value;
    for_each_index<tNumberOfMembers>(CollectBoostFusionMemberNames{tMemberNames});
    const auto tExpectedMemberNames = std::vector<std::string>{"mInteger", "mDouble", "mString"};
    EXPECT_EQ(tMemberNames, tExpectedMemberNames);
}
}  // namespace plato::utilities::unittest
