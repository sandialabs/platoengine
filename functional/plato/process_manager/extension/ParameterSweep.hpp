#ifndef PLATO_PROCESSMANAGER_EXTENSION_PARAMETERSWEEP
#define PLATO_PROCESSMANAGER_EXTENSION_PARAMETERSWEEP

#include <filesystem>
#include <optional>
#include <string>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/Range.hpp"
#include "plato/input_validation/ValidatedInputTypeWrapper.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/process_manager/extension/IndexAndRange.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/utilities/LinearSpaceGenerator.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::input_parser
{
using IndexAndRangeList = AutoList<IndexAndRange>;
struct parameter_sweep;
}  // namespace plato::input_parser

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), parameter_sweep,
    (plato::input_parser::FileName, output_file_name, "Required filename specifying where to output the results.")
    (plato::input_parser::IndexAndRangeList, indices_and_ranges,"Required input specifying a comma separated list of indices and ranges."
                                                              " e.g. index 0 range [0:1:2], index 1 range [-1:1:1]  Runs variable at index 0 from 0 to 2 with a step of 1, cross the space with variable 1 going from -1 to 1 by 1.")
)
// clang-format on

namespace plato::process_manager::extension
{
using Domain = utilities::NamedType<std::vector<double>, struct DomainTag>;
using Values = utilities::NamedType<std::vector<double>, struct ValuesTag>;

/// @brief An implementation of a process manager that performs a parameter sweep
class ParameterSweep
{
   public:
    ///@brief Construct a ParameterSweep from the given processor manager input @a aInput
    explicit ParameterSweep(const library::ValidatedProcessManagerInput& aInput);

    ///@brief Run the parameter sweep on the problem defined in @a aProcessManagerData
    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    using Parameters = std::pair<std::vector<std::size_t>, std::vector<utilities::LinearSpaceGenerator>>;

    std::filesystem::path mOutputFileName;
    Parameters mParameters = {};
};

namespace detail
{

[[nodiscard]] auto validate_output_file_name(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_indices_and_ranges(const input_parser::parameter_sweep& aInput)
    -> std::optional<std::string>;
[[nodiscard]] auto validate_range_limits(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_range_steps(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_control_indices(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_control_indices_no_duplicates(const input_parser::parameter_sweep& aInput)
    -> std::optional<std::string>;

void output_table(const std::filesystem::path& aPath, const Domain& aDomain, const Values& aValues);

[[nodiscard]] auto extract_parameters(const std::vector<input_parser::IndexAndRange>& aIndexAndRanges)
    -> std::pair<std::vector<std::size_t>, std::vector<utilities::LinearSpaceGenerator>>;

}  // namespace detail

}  // namespace plato::process_manager::extension
#endif
