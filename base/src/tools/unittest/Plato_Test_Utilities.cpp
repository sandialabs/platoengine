#include "Plato_ParseCSMUtilities.hpp"
#include "Plato_FreeFunctions.hpp"

#include <gtest/gtest.h>

namespace PlatoTestUtilityFunctions
{

TEST(ParseCSMUtilities, skipLinesWithComments)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "# Constant, Design, and Output Parameters:\n"
            "# despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "# despmtr Param 3.20000 lbound 5.1 ubound 9.0 initial 6.2\n"
            "despmtr Param 0.20000 lbound 0.1 ubound 0.5 initial 0.2\n"
            "# despmtr Param 6.20000 lbound 4.1 ubound 9.0 initial 5.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds);

    EXPECT_EQ(tInitialValues.size(), 1);
    EXPECT_EQ(tLowerBounds.size(), 1);
    EXPECT_EQ(tUpperBounds.size(), 1);

    EXPECT_EQ(tInitialValues[0], 0.2);
    EXPECT_EQ(tLowerBounds[0], 0.1);
    EXPECT_EQ(tUpperBounds[0], 0.5);
}

TEST(ParseCSMUtilities, handleVariableSpacing)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "# Constant, Design, and Output Parameters:\n"
            "despmtr  Param  0.20000  lbound  0.134  ubound  88  initial  4.5\n"
            "despmtr            Param  0.20000 lbound   0 ubound 4 initial 0.2\n"
            "despmtr    Param   0.20000     lbound  2.0     ubound      77.4 initial                 3.1\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds);

    EXPECT_EQ(tInitialValues.size(), 3);
    EXPECT_EQ(tLowerBounds.size(), 3);
    EXPECT_EQ(tUpperBounds.size(), 3);

    EXPECT_EQ(tInitialValues[0], 4.5);
    EXPECT_EQ(tLowerBounds[0], 0.134);
    EXPECT_EQ(tUpperBounds[0], 88.0);

    EXPECT_EQ(tInitialValues[1], 0.2);
    EXPECT_EQ(tLowerBounds[1], 0.0);
    EXPECT_EQ(tUpperBounds[1], 4.0);

    EXPECT_EQ(tInitialValues[2], 3.1);
    EXPECT_EQ(tLowerBounds[2], 2.0);
    EXPECT_EQ(tUpperBounds[2], 77.4);
}

TEST(ParseCSMUtilities, handleVariableOrdering)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "# Constant, Design, and Output Parameters:\n"
            "despmtr Param 0.20000 lbound 0.134 ubound 88 initial 4.5\n"
            "despmtr Param 0.20000 lbound 0.14 initial 1.2 ubound 8.81\n"
            "despmtr Param 0.20000 initial 0.2 lbound 0 ubound 4\n"
            "despmtr Param 0.20000 initial 24.2 ubound 88.0 lbound 14.2\n"
            "despmtr Param 0.20000 ubound 77.4 initial 4.0 lbound 3.1\n"
            "despmtr Param 0.20000 ubound 17.14 lbound -12.3 initial -3.1\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds);

    EXPECT_EQ(tInitialValues.size(), 6);
    EXPECT_EQ(tLowerBounds.size(), 6);
    EXPECT_EQ(tUpperBounds.size(), 6);

    EXPECT_EQ(tInitialValues[0], 4.5);
    EXPECT_EQ(tLowerBounds[0], 0.134);
    EXPECT_EQ(tUpperBounds[0], 88.0);

    EXPECT_EQ(tInitialValues[1], 1.2);
    EXPECT_EQ(tLowerBounds[1], 0.14);
    EXPECT_EQ(tUpperBounds[1], 8.81);

    EXPECT_EQ(tInitialValues[2], 0.2);
    EXPECT_EQ(tLowerBounds[2], 0.0);
    EXPECT_EQ(tUpperBounds[2], 4.0);

    EXPECT_EQ(tInitialValues[3], 24.2);
    EXPECT_EQ(tLowerBounds[3], 14.2);
    EXPECT_EQ(tUpperBounds[3], 88.0);

    EXPECT_EQ(tInitialValues[4], 4.0);
    EXPECT_EQ(tLowerBounds[4], 3.1);
    EXPECT_EQ(tUpperBounds[4], 77.4);

    EXPECT_EQ(tInitialValues[5], -3.1);
    EXPECT_EQ(tLowerBounds[5], -12.3);
    EXPECT_EQ(tUpperBounds[5], 17.14);
}

TEST(ParseCSMUtilities, errorNoDespmtrKeyword)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            " Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "design_parameter Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "design parameter Param 0.20000 lbound 0.1 ubound 0.5 initial 0.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, errorNoInitialKeyword)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, errorNoLboundKeyword)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param 1.20000 6.1 ubound 9.0 initial 7.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, errorNoUboundKeyword)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param 1.20000 lbound 6.1 9.0 initial 7.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, errorLboundGreaterThanUbound)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param 1.20000 lbound 61.1 ubound 9.0 initial 71.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, errorInitialLessThanLbound)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 3.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, errorInitialGreaterThanUbound)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param 1.20000 lbound 6.1 ubound 9.0 initial 13.2\n";
    tStringStream.str(tStringInput);

    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    ASSERT_THROW(Plato::ParseCSM::getValuesFromStream(tStringStream, tInitialValues, tLowerBounds, tUpperBounds), std::runtime_error);
}

TEST(ParseCSMUtilities, getCorrectDescriptors)
{
    std::istringstream tStringStream;
    std::string tStringInput;

    tStringInput =
            "despmtr Param1 1.20000 lbound 6.1 ubound 9.0 initial 7.2\n"
            "despmtr Param2 1.20000 lbound 6.1 ubound 9.0 initial 13.2\n";
    tStringStream.str(tStringInput);

    std::vector<std::string> tDescriptors;
    Plato::ParseCSM::getDescriptorsFromStream(tStringStream, tDescriptors);

    EXPECT_EQ(tDescriptors.size(), 2);
    EXPECT_EQ(tDescriptors[0], "Param1");
    EXPECT_EQ(tDescriptors[1], "Param2");
}

TEST(FreeFunctions, stripSpaces)
{
    EXPECT_EQ(Plato::stripSpaces("  Hello    World    "), "HelloWorld");
    EXPECT_EQ(Plato::stripSpaces("HelloWorld"), "HelloWorld");
    EXPECT_NE(Plato::stripSpaces("Hello World"), "Hello World");
}

} 