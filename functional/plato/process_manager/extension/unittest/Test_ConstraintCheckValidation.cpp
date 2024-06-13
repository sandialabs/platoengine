#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ValidateConstraintCheck, ValidateLinearityCheckOutputFileName)
{
    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_linearity_check_output_file_name(tConstraintCheck).has_value());
    tConstraintCheck.linearity_check_output_file_name = input_parser::FileName{"empty.txt"};
    EXPECT_FALSE(detail::validate_linearity_check_output_file_name(tConstraintCheck).has_value());
}

TEST(ValidateConstraintCheck, ValidateJacobianCheckOutputFileName)
{
    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_jacobian_check_output_file_name(tConstraintCheck).has_value());
    tConstraintCheck.jacobian_check_output_file_name = input_parser::FileName{"empty.txt"};
    EXPECT_FALSE(detail::validate_jacobian_check_output_file_name(tConstraintCheck).has_value());
}

TEST(ValidateConstraintCheck, ValidateJacobianAdjointConsistencyOutputFileName)
{
    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_jacobian_adjoint_consistency_check_output_file_name(tConstraintCheck).has_value());
    tConstraintCheck.jacobian_adjoint_consistency_output_file_name = input_parser::FileName{"empty.txt"};
    EXPECT_FALSE(detail::validate_jacobian_adjoint_consistency_check_output_file_name(tConstraintCheck).has_value());
}

}  // namespace plato::process_manager::extension::unittest