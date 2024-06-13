#ifndef PLATO_PROCESSMANAGER_EXTENSION_GRADIENTCHECK
#define PLATO_PROCESSMANAGER_EXTENSION_GRADIENTCHECK

#include <filesystem>
#include <optional>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::input_parser
{
struct gradient_check;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{
/// @brief An implementation of a process manager that performs a gradient check using
///  ROL's gradient check functionality.
class GradientCheck
{
   public:
    using ValidatedGradientCheckInput = core::ValidatedInputTypeWrapper<input_parser::gradient_check>;

    explicit GradientCheck(const ValidatedGradientCheckInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    std::filesystem::path mOutputFileName;
    unsigned int mNumberOfSteps = 12;
    double mInitialDirectionMagnitude = 1;
    double mStepSizeReductionFactor = 0.1;
    unsigned int mRandomDirectionSeed = 42;
};

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_output_file_name(const input_parser::gradient_check& aInput);

}  // namespace detail

}  // namespace plato::process_manager::extension
#endif
