#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLCONSTRAINTCHECK
#define PLATO_PROCESSMANAGER_EXTENSION_ROLCONSTRAINTCHECK

#include <filesystem>
#include <optional>
#include <string>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::input_parser
{
struct constraint_check;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), constraint_check,
    (plato::input_parser::FileName, linearity_check_output_file_name, "Required filename to use to report the results of the linearity check.")
    (plato::input_parser::FileName, jacobian_check_output_file_name, "Required filename to use to report the results of the Jacobian check.")
    (plato::input_parser::FileName, jacobian_adjoint_consistency_output_file_name, "Required filename to use to report the results of the Jacobian and adjoint Jacobian consistency check.")
    (unsigned int, number_of_steps, "Required field specifying the number of approximations to evaluate using a finite difference.")
    (double, initial_direction_magnitude, "Required field specifying the magnitude of the perturbation of the design controls.")
    (double, step_size_reduction_factor, "Required field specifying how much the perturbation will be reduced for each step, e.g., 0.1 for log10 step sizes.")
    (unsigned int, random_direction_seed, "Required field specifying the seed that is used to generate the random perturbation of the controls.")
)
// clang-format on

namespace plato::process_manager::extension
{
/// @brief An implementation of process manager that performs a constraint check
/// using the ROL utilities checkApplyJacobian, checkAdjointConsistencyJacobian, and CheckLinearity
class ConstraintCheck
{
   public:
    explicit ConstraintCheck(const library::ValidatedProcessManagerInput& aInput);

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
[[nodiscard]] auto validate_linearity_check_output_file_name(const input_parser::constraint_check& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_jacobian_check_output_file_name(const input_parser::constraint_check& aInput)
    -> std::optional<std::string>;

[[nodiscard]] auto validate_jacobian_adjoint_consistency_check_output_file_name(
    const input_parser::constraint_check& aInput) -> std::optional<std::string>;

void write_jacobian_adjoint_consistency_check_output(const std::string& aName, double aTolerance);
}  // namespace detail

}  // namespace plato::process_manager::extension

#endif
