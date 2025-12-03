#include <gtest/gtest.h>

#include <cstddef>

#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(DirectionVectorTypes, EnumTable)
{
    EXPECT_TRUE(input_parser::kDirectionVectorTypesTable.toString(input_parser::DirectionVectorTypes::kRandom));
    EXPECT_EQ(input_parser::kDirectionVectorTypesTable.toString(input_parser::DirectionVectorTypes::kRandom).value(),
              "random");

    EXPECT_TRUE(
        input_parser::kDirectionVectorTypesTable.toString(input_parser::DirectionVectorTypes::kUniformNegative));
    EXPECT_EQ(
        input_parser::kDirectionVectorTypesTable.toString(input_parser::DirectionVectorTypes::kUniformNegative).value(),
        "uniform_negative");
    EXPECT_TRUE(
        input_parser::kDirectionVectorTypesTable.toString(input_parser::DirectionVectorTypes::kUniformPositive));
    EXPECT_EQ(
        input_parser::kDirectionVectorTypesTable.toString(input_parser::DirectionVectorTypes::kUniformPositive).value(),
        "uniform_positive");
}

TEST(DirectionVectorTypes, MakeUniformOrRandomPerturbation)
{
    constexpr auto tComponentValue = 10.0;
    constexpr auto tSize = std::size_t{2};
    auto tROLStdVector = ROL::StdVector<double>{tComponentValue, tComponentValue};
    EXPECT_NE(tROLStdVector.norm(), 1.0);

    make_uniform_or_random_perturbation(tROLStdVector, input_parser::DirectionVectorTypes::kRandom);
    EXPECT_DOUBLE_EQ(tROLStdVector.norm(), 1.0);
    EXPECT_NE(tROLStdVector.getVector()->front(), tComponentValue);

    make_uniform_or_random_perturbation(tROLStdVector, input_parser::DirectionVectorTypes::kUniformPositive);
    EXPECT_EQ(*tROLStdVector.getVector(), std::vector(tSize, 1.0));

    make_uniform_or_random_perturbation(tROLStdVector, input_parser::DirectionVectorTypes::kUniformNegative);
    EXPECT_EQ(*tROLStdVector.getVector(), std::vector(tSize, -1.0));
}

}  // namespace plato::process_manager::extension::unittest
