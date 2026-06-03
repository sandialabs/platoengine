#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>
#include <ROL_LineSearchStep.hpp>
#include <ROL_ParameterList.hpp>
#include <ROL_Ptr.hpp>
#include <ROL_StatusTest.hpp>
#include <ROL_StdVector.hpp>
#include <iostream>
#include <ostream>

#include "plato/core/Compose.hpp"
#include "plato/integration_tests/utilities/DynamicVectorPenaltyFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::integration_tests::serial
{
namespace
{
constexpr bool kPrintFlag = true;

[[nodiscard]] auto create_parameter_list() -> ROL::ParameterList
{
    ROL::ParameterList tParlist;
    constexpr auto tGradientTolerance = 1e-12;
    tParlist.sublist("Status Test").set("Gradient Tolerance", tGradientTolerance);
    constexpr auto tStepTolerance = 1e-14;
    tParlist.sublist("Status Test").set("Step Tolerance", tStepTolerance);
    constexpr auto tIterationLimit = 100;
    tParlist.sublist("Status Test").set("Iteration Limit", tIterationLimit);

    return tParlist;
}

[[nodiscard]] auto rol_algorithm() -> ROL::Algorithm<double>
{
    // Set parameters.
    ROL::ParameterList tParlist = create_parameter_list();

    // Define algorithm
    auto tStep = ROL::makePtr<ROL::LineSearchStep<double>>(tParlist);
    auto tStatus = ROL::makePtr<ROL::StatusTest<double>>(tParlist);
    return ROL::Algorithm<double>{tStep, tStatus, false};
}

}  // namespace

TEST(Optimize, Rosenbrock)
{
    const ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    constexpr auto tInitialGuessX = -1.2;
    constexpr auto tInitialGuessY = 1.0;
    auto tControl = ROL::StdVector<double>{tInitialGuessX, tInitialGuessY};
    auto tObjective = third_party_integration::rol::ROLObjectiveFunction{
        utilities::make_rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{})};

    rol_algorithm().run(tControl, tObjective, kPrintFlag, *tOutStream);

    constexpr double tXMinValue = 1.0;
    constexpr double tTolerance = 1e-14;
    EXPECT_NEAR(tControl[0], tXMinValue, tTolerance);
    EXPECT_NEAR(tControl[1], tXMinValue, tTolerance);
}

TEST(Optimize, RosenbrockPenaltyComposition)
{
    const ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    constexpr auto tInitialGuessX = 1.5;
    constexpr auto tInitialGuessY = 0.5;
    auto tControl = ROL::StdVector<double>{tInitialGuessX, tInitialGuessY};
    constexpr auto tXMin = test_utilities::XMin{0.0};
    constexpr auto tPower = test_utilities::Exponent{3.0};
    auto tObjective = third_party_integration::rol::ROLObjectiveFunction{
        core::compose(utilities::make_rosenbrock_dynamic_vector_function(test_utilities::Rosenbrock{}),
                      utilities::make_penalty_dynamic_vector_function(test_utilities::Penalty{tXMin, tPower}))};

    rol_algorithm().run(tControl, tObjective, kPrintFlag, *tOutStream);

    const double tXMinValue = 1.0;
    EXPECT_DOUBLE_EQ(tControl[0], tXMinValue);
    EXPECT_DOUBLE_EQ(tControl[1], tXMinValue);
}
}  // namespace plato::integration_tests::serial
