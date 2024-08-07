#include <gtest/gtest.h>

#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::third_party_integration::rol::unittest
{
TEST(ROLUtilities, ToDynamicVector)
{
    const auto tROLStdVector = ROL::StdVector<double>{1.0, 2.0, 3.0};
    const ROL::StdVector<double>& tROLVector = tROLStdVector;
    const linear_algebra::DynamicVector<double> tDynamicVector = to_dynamic_vector(tROLVector);
    EXPECT_EQ(tDynamicVector.stdVector(), *tROLStdVector.getVector());
}

TEST(ROLUtilities, ToROLVector)
{
    const auto tDynamicVector = linear_algebra::DynamicVector<double>{1.0, 2.0, 3.0};
    const ROL::StdVector<double> tROLStdVector = to_rol_vector(tDynamicVector);
    EXPECT_EQ(tDynamicVector.stdVector(), *tROLStdVector.getVector());
}

TEST(ROLUtilities, MakeROLVector)
{
    const auto tDynamicVector = linear_algebra::DynamicVector<double>{1.0, 2.0, 3.0};
    const auto tROLStdVector = make_rol_vector(tDynamicVector);
    EXPECT_EQ(tDynamicVector.stdVector(), *tROLStdVector->getVector());
}

TEST(ROLUtilities, AssignVector)
{
    const auto tVector = std::vector{1.0, 2.0, 3.0};
    auto tROLStdVector = ROL::StdVector<double>{-1.0, -2.0};
    assign_vector(tROLStdVector, tVector);
    EXPECT_EQ(*tROLStdVector.getVector(), tVector);
}

TEST(ROLUtilities, GeneratePerturbation)
{
    constexpr int tDimensions = 3;
    const auto tVector = generate_perturbation(tDimensions);
    EXPECT_EQ(tVector.dimension(), tDimensions);
    EXPECT_DOUBLE_EQ(tVector.norm(), 1.0);
}

}  // namespace plato::third_party_integration::rol::unittest
