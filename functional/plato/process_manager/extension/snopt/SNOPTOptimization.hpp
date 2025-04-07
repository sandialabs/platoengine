#ifndef PLATO_PROCESSMANAGER_EXTENSION_SNOPT_SNOPTOPTIMIZATION
#define PLATO_PROCESSMANAGER_EXTENSION_SNOPT_SNOPTOPTIMIZATION

#include <filesystem>
#include <optional>
#include <string>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"
#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

namespace plato::input_parser
{
struct snopt_optimization;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), new_snopt_optimization,
    (plato::input_parser::FileName, input_file_name, "Optional filename of the SNOpt input file to set optimization parameters not otherwise available here.")
    (unsigned int, max_iterations, "Optional command to override the maximum number of outer iterations given in an input file.")
    (unsigned int, time_limit_in_minutes, "Optional command to override the time limit in minutes for this optimization as an additional stopping criteria for the optimization.")
    (bool, output_design_history, "Optional command to write the entire design history to output file. Default is to only write the final design.")
)
// clang-format on

namespace plato::process_manager::extension::snopt
{
/// @brief An implementation of a process manager that performs optimization using SNOPT.
class SNOPTOptimization
{
   public:
    using ValidatedOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::snopt_optimization>;

    explicit SNOPTOptimization(const ValidatedOptimizationParameters& aInput);
    explicit SNOPTOptimization(const library::NewValidatedProcessManagerInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    std::filesystem::path mOutputLogFileName;
    third_party_integration::snopt::SNOPTOptions mOptions;
};

/// @brief Creates a valid example SNOPTOptimization input struct, useful for tests.
[[nodiscard]] auto create_valid_example_snopt_optimization_input() -> input_parser::new_snopt_optimization;

namespace detail
{
[[nodiscard]] auto make_constraints(const library::ProcessManagerData& aProcessManagerData)
    -> third_party_integration::snopt::InterfaceConstraintVectorType;

[[nodiscard]] auto validate_time_limit_in_minutes(const input_parser::new_snopt_optimization& aInput)
    -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::process_manager::extension::snopt

#endif
