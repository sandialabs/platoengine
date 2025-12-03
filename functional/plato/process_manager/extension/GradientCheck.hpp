#ifndef PLATO_PROCESSMANAGER_EXTENSION_GRADIENTCHECK
#define PLATO_PROCESSMANAGER_EXTENSION_GRADIENTCHECK

#include <filesystem>
#include <optional>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::input_parser
{
struct gradient_check;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), gradient_check,
    (plato::input_parser::FileName, output_file_name, "Required filename to use to report the results of the gradient check.")
    (unsigned int, number_of_steps, "Required field specifying the number of approximations to evaluate using a finite difference.")
    (double, initial_direction_magnitude, "Required field specifying the magnitude of the perturbation of the design controls.")
    (double, step_size_reduction_factor, "Required field specifying how much the perturbation will be reduced for each step, e.g., 0.1 for log10 step sizes.")
    (unsigned int, random_direction_seed, "Optional field specifying the seed that is used to generate the random perturbation of the controls. "
                                                    "If no seed is provided, the clock will be used to generate a seed.")
    (plato::input_parser::DirectionVectorTypes, direction_vector_type, "Required field specifying how to generate the direction vector. "
                                                                       "Inputs can be: 'random', 'uniform_positive', or 'uniform_negative'")                                                    
)
// clang-format on

namespace plato::process_manager::extension
{
/// @brief An implementation of a process manager that performs a gradient check using
///  ROL's gradient check functionality.
class GradientCheck
{
   public:
    explicit GradientCheck(const library::ValidatedProcessManagerInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    std::filesystem::path mOutputFileName;
    unsigned int mNumberOfSteps = 12;
    double mInitialDirectionMagnitude = 1;
    double mStepSizeReductionFactor = 0.1;
    input_parser::DirectionVectorTypes mDirectionVectorType = input_parser::DirectionVectorTypes::kRandom;
    unsigned int mSeed = 42;
};

namespace detail
{

/// @brief Validate that an output_file_name is specified in the input @a aInput
[[nodiscard]] auto validate_output_file_name(const input_parser::gradient_check& aInput) -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::process_manager::extension
#endif
