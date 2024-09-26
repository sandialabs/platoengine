#include <gtest/gtest.h>

#include "plato/third_party_integration/snopt/ObjectiveConstraintArrayView.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
constexpr auto kObjectiveArrayIndex = ObjectiveConstraintArrayView<int>::objectiveArrayIndex();
constexpr auto kNumberOfObjectives = std::size_t{1};

[[nodiscard]] constexpr auto array_size(const ConstraintSizeType aNumberOfConstraints) -> std::size_t
{
    return aNumberOfConstraints.mValue + kNumberOfObjectives;
}
}  // namespace

TEST(ObjectiveConstraintArrayView, ConstObjective)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{3};
    const auto tArray = std::make_unique<int[]>(array_size(tNumberOfConstraints));

    constexpr auto tExpectedObjective = 42;
    tArray[kObjectiveArrayIndex] = tExpectedObjective;
    const auto tObjectiveConstraintArray = ObjectiveConstraintArrayView{tArray.get(), tNumberOfConstraints};

    EXPECT_EQ(tObjectiveConstraintArray.objective(), tExpectedObjective);
}

TEST(ObjectiveConstraintArrayView, Objective)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{3};
    const auto tArray = std::make_unique<int[]>(array_size(tNumberOfConstraints));
    auto tObjectiveConstraintArray = ObjectiveConstraintArrayView{tArray.get(), tNumberOfConstraints};

    constexpr auto tExpectedObjective = 13;
    tObjectiveConstraintArray.objective() = tExpectedObjective;

    EXPECT_EQ(tArray[kObjectiveArrayIndex], tExpectedObjective);
}

TEST(ObjectiveConstraintArrayView, ConstConstraintIterators)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{3};
    const auto tArray = test_utilities::array_with_iota<int>(array_size(tNumberOfConstraints));
    const auto tObjectiveConstraintArray = ObjectiveConstraintArrayView{tArray.get(), tNumberOfConstraints};

    EXPECT_EQ(
        std::distance(tObjectiveConstraintArray.constraints().begin(), tObjectiveConstraintArray.constraints().end()),
        tNumberOfConstraints.mValue);

    EXPECT_EQ(tObjectiveConstraintArray.constraints().begin(), tArray.get() + kNumberOfObjectives);
    EXPECT_EQ(tObjectiveConstraintArray.constraints().end(), tArray.get() + array_size(tNumberOfConstraints));

    auto tVectorFromIterators = std::vector<int>(tNumberOfConstraints.mValue);
    std::copy(tObjectiveConstraintArray.constraints().begin(), tObjectiveConstraintArray.constraints().end(),
              tVectorFromIterators.begin());
    for (const auto tIndex : utilities::IndexRange{tNumberOfConstraints.mValue})
    {
        EXPECT_EQ(tVectorFromIterators[tIndex], tArray[tIndex + kNumberOfObjectives]);
    }
}

TEST(ObjectiveConstraintArrayView, ConstraintIterators)
{
    constexpr auto tNumberOfConstraints = ConstraintSizeType{3};
    constexpr auto tSize = array_size(tNumberOfConstraints);
    const auto tArray = std::make_unique<int[]>(tSize);
    auto tObjectiveConstraintArray = ObjectiveConstraintArrayView{tArray.get(), tNumberOfConstraints};

    std::iota(tObjectiveConstraintArray.constraints().begin(), tObjectiveConstraintArray.constraints().end(), 0);

    for (const auto tIndex : utilities::IndexRange{tNumberOfConstraints.mValue})
    {
        EXPECT_EQ(tArray[tIndex + kNumberOfObjectives], tIndex);
    }
}

}  // namespace plato::third_party_integration::snopt::unittest
