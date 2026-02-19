#include <gtest/gtest.h>

#include <fstream>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
template <typename Criteria>
void check_validation_app(const test_utilities::TestContext& aTestContext)
{
    Criteria tCriteria;
    EXPECT_TRUE(detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    tCriteria.app = input_parser::AppName{"definitely not an app"};
    EXPECT_TRUE(detail::validate_criterion_is_registered(tCriteria).has_value()) << aTestContext;

    EXPECT_FALSE(detail::validate_number_of_processors(tCriteria).has_value()) << aTestContext;
    tCriteria.number_of_processors = 0;
    EXPECT_TRUE(detail::validate_number_of_processors(tCriteria).has_value()) << aTestContext;
    tCriteria.number_of_processors = 1;
    EXPECT_FALSE(detail::validate_number_of_processors(tCriteria).has_value()) << aTestContext;
}

void create_dummy_file(const std::filesystem::path& aFile)
{
    std::ofstream tOutFile(aFile);
    tOutFile << "Gaba ghoul" << std::endl;
}

template <typename Criteria>
void check_file_validation(const test_utilities::TestContext& aTestContext)
{
    const auto tFileOne = std::filesystem::path{"file.txt"};
    const auto tFileTwo = std::filesystem::path{"aux.txt"};
    create_dummy_file(tFileOne);
    Criteria tCriteria;
    EXPECT_FALSE(detail::validate_criterion_files_exist(tCriteria).has_value())
        << aTestContext << " Valid - no files specified.";
    tCriteria.input_files = input_parser::FileList{{tFileOne, tFileTwo}};
    EXPECT_TRUE(detail::validate_criterion_files_exist(tCriteria).has_value())
        << aTestContext << " Invalid - missing 1 of the files specified.";
    create_dummy_file(tFileTwo);
    EXPECT_FALSE(detail::validate_criterion_files_exist(tCriteria).has_value()) << aTestContext << " Valid.";
    std::filesystem::remove(tFileOne);
    std::filesystem::remove(tFileTwo);
}

}  // namespace

TEST(CriterionValidation, CheckValidationAppAndCustomAppOnObjective)
{
    check_validation_app<input_parser::objective>(TEST_CONTEXT("Objective"));
}

TEST(CriterionValidation, CheckValidationAppAndCustomAppOnConstraint)
{
    check_validation_app<input_parser::constraint>(TEST_CONTEXT("Constraint"));
}

TEST(CriterionValidation, CheckValidationCriterionFilesExistOnObjective)
{
    check_file_validation<input_parser::objective>(TEST_CONTEXT("Objective"));
}

TEST(CriterionValidation, CheckValidationCriterionFilesExistOnConstraint)
{
    check_file_validation<input_parser::constraint>(TEST_CONTEXT("Constraint"));
}
}  // namespace plato::criteria::library::unittest
