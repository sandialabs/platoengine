#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>
#include <ROL_LineSearchStep.hpp>
#include <ROL_StatusTest.hpp>
#include <ROL_StdVector.hpp>
#include <ROL_Stream.hpp>
#include <Teuchos_GlobalMPISession.hpp>

#include "plato/core/Compose.hpp"
#include "plato/integration_tests/utilities/DynamicVectorPenaltyFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/rol_integration/ROLObjectiveFunction.hpp"

namespace plato::integration_tests::serial
{
namespace
{
constexpr bool tPrintFlag = true;

[[nodiscard]] ROL::Algorithm<double> rol_algorithm()
{
    // Set parameters.
    ROL::ParameterList parlist;
    parlist.sublist("Step").sublist("Line Search").sublist("Descent Method").set("Type", "Newton-Krylov");
    parlist.sublist("Status Test").set("Gradient Tolerance", 1.e-12);
    parlist.sublist("Status Test").set("Step Tolerance", 1.e-14);
    parlist.sublist("Status Test").set("Iteration Limit", 100);

    // Define algorithm
    auto step = ROL::makePtr<ROL::LineSearchStep<double>>(parlist);
    auto status = ROL::makePtr<ROL::StatusTest<double>>(parlist);
    return ROL::Algorithm<double>{step, status, false};
}
}  // namespace

TEST(Optimize, Rosenbrock)
{
    namespace pft = plato::test_utilities;

    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    auto tControl = ROL::StdVector<double>{-1.2, 1.0};
    auto tObjective = plato::rol_integration::ROLObjectiveFunction{
        utilities::make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{})};

    rol_algorithm().run(tControl, tObjective, tPrintFlag, *tOutStream);

    const double tXMinValue = 1.0;
    EXPECT_EQ(tControl[0], tXMinValue);
    EXPECT_EQ(tControl[1], tXMinValue);
}

TEST(Optimize, RosenbrockPenaltyComposition)
{
    namespace pft = plato::test_utilities;

    ROL::Ptr<std::ostream> tOutStream = ROL::makePtrFromRef(std::cout);
    auto tControl = ROL::StdVector<double>{1.5, 0.5};
    constexpr double tXMin = 0.0;
    constexpr double tPower = 3.0;
    auto tObjective = plato::rol_integration::ROLObjectiveFunction{
        core::compose(utilities::make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{}),
                      utilities::make_penalty_dynamic_vector_function(pft::Penalty{tXMin, tPower}))};

    rol_algorithm().run(tControl, tObjective, tPrintFlag, *tOutStream);

    const double tXMinValue = 1.0;
    EXPECT_EQ(tControl[0], tXMinValue);
    EXPECT_EQ(tControl[1], tXMinValue);
}
}  // namespace plato::integration_tests::serial
