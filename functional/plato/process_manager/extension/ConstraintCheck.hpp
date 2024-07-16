#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLCONSTRAINTCHECK
#define PLATO_PROCESSMANAGER_EXTENSION_ROLCONSTRAINTCHECK

#include <filesystem>
#include <optional>
#include <string>

#include "plato/core/ValidatedInputTypeWrapper.hpp"

namespace plato::input_parser
{
struct constraint_check;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{
/// @brief An implementation of process manager that performs a constraint check
/// using the ROL utilities checkApplyJacobian, checkAdjointConsistencyJacobian, and CheckLinearity
class ConstraintCheck
{
    using ValidatedConstraintCheckInput = core::ValidatedInputTypeWrapper<input_parser::constraint_check>;

   public:
    explicit ConstraintCheck(const ValidatedConstraintCheckInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    std::filesystem::path mLinearityCheckOutputFileName;
    std::filesystem::path mJacobianCheckOutputFileName;
    std::filesystem::path mJacobianAdjointConsistencyCheckOutputFileName;
    unsigned int mNumberOfSteps = 12;
    double mInitialDirectionMagnitude = 1;
    double mStepSizeReductionFactor = 0.1;
    unsigned int mRandomDirectionSeed = 123;
};

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_linearity_check_output_file_name(
    const input_parser::constraint_check& aInput);

[[nodiscard]] std::optional<std::string> validate_jacobian_check_output_file_name(
    const input_parser::constraint_check& aInput);

[[nodiscard]] std::optional<std::string> validate_jacobian_adjoint_consistency_check_output_file_name(
    const input_parser::constraint_check& aInput);

void write_jacobian_adjoint_consistency_check_output(const std::string& aName, double aTolerance);
}  // namespace detail

}  // namespace plato::process_manager::extension

#endif
