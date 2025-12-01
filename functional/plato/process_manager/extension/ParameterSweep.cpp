#include "plato/process_manager/extension/ParameterSweep.hpp"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <set>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/extension/CombinationGenerator.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerLogger.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"
#include "plato/utilities/LinearSpaceGenerator.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::process_manager::extension
{
namespace
{
constexpr auto kWidth = std::size_t{32};
constexpr auto kPrecision = std::size_t{16};
using FixedFormatNumber = utilities::FixedWidthFloatingPointOutput<double, kPrecision, kWidth>;

[[nodiscard]] auto make_parameter_sweep_process_manager(const library::ValidatedProcessManagerInput& aValidInput)
    -> library::StageAndProcessManager
{
    return {library::RunStage::kExecute, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            { ParameterSweep{aValidInput}.run(aProcessManangerData); }};
}

[[maybe_unused]] static auto kParameterSweepParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::parameter_sweep>{};

[[maybe_unused]] static auto kParameterSweepProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::parameter_sweep>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_parameter_sweep_process_manager(aValidInput); }};

[[maybe_unused]] static auto kParameterSweepValidationRegistration =
    input_validation::ValidationRegistration<input_parser::parameter_sweep>{
        [](const input_parser::parameter_sweep& aInput) { return detail::validate_output_file_name(aInput); },
        [](const input_parser::parameter_sweep& aInput) { return detail::validate_indices_and_ranges(aInput); },
        [](const input_parser::parameter_sweep& aInput) { return detail::validate_control_indices(aInput); },
        [](const input_parser::parameter_sweep& aInput)
        { return detail::validate_control_indices_no_duplicates(aInput); },
        [](const input_parser::parameter_sweep& aInput) { return detail::validate_range_limits(aInput); },
        [](const input_parser::parameter_sweep& aInput) { return detail::validate_range_steps(aInput); }};

[[nodiscard]] auto parameter_sweep_input(const library::ValidatedProcessManagerInput& aValidInput)
    -> const input_parser::parameter_sweep&
{
    return input_validation::get_input_block<input_parser::parameter_sweep>(aValidInput);
}

}  // namespace

ParameterSweep::ParameterSweep(const library::ValidatedProcessManagerInput& aInput)
    : mOutputFileName(parameter_sweep_input(aInput).output_file_name.value().mToken),
      mParameters(detail::extract_parameters(parameter_sweep_input(aInput).indices_and_ranges.value().mList))
{
}

namespace
{

void write_table_header(const std::vector<std::size_t>& aIndices, std::ofstream& aOutFile)
{
    std::for_each(aIndices.begin(), aIndices.end(),
                  [&aOutFile](const auto aIndex)
                  {
                      const auto tControlString = "Control:" + std::to_string(aIndex);
                      aOutFile << std::setw(kWidth + 1) << tControlString;
                  });
    aOutFile << std::setw(kWidth) << "Value" << std::endl;
}

void write_table_row(const std::vector<double>& aControls, const double aValue, std::ofstream& aOutFile)
{
    std::for_each(aControls.begin(), aControls.end(),
                  [&aOutFile](const auto aControl) { aOutFile << std::setw(kWidth) << aControl << ","; });
    aOutFile << std::setw(kWidth) << aValue << std::endl;
}

[[nodiscard]] auto transform_to_steps_vectors(
    const std::vector<utilities::LinearSpaceGenerator>& aLinearSpaceGenerators) -> std::vector<std::vector<double>>
{
    std::vector<std::vector<double>> tStepsVector;
    tStepsVector.reserve(aLinearSpaceGenerators.size());
    std::ranges::transform(aLinearSpaceGenerators, std::back_inserter(tStepsVector),
                           [](const auto aLinearSpaceGenerator) { return aLinearSpaceGenerator.steps(); });
    return tStepsVector;
}

[[nodiscard]] auto down_select_to_sub_entries(const std::vector<double>& aValues,
                                              const std::vector<std::size_t>& aSubIndices) -> std::vector<double>
{
    std::vector<double> tDownSelect;
    tDownSelect.reserve(aSubIndices.size());
    std::ranges::transform(aSubIndices, std::back_inserter(tDownSelect),
                           [&aValues](const auto aSubIndex) { return aValues[aSubIndex]; });
    return tDownSelect;
}

}  // namespace

void ParameterSweep::run(const library::ProcessManagerData& aProcessManagerData) const
{
    [[maybe_unused]] const auto tTaskLogger = library::run_task_log<input_parser::parameter_sweep>();
    const auto tInitialGuess = aProcessManagerData.mGeometry.mInitialGuess.stdVector();
    const auto tObjective = core::compose(aProcessManagerData.mObjective, aProcessManagerData.mGeometry.mCompute);

    const auto& [tSubIndices, tLinearSpaceGenerators] = mParameters;
    const auto tStepsVector = transform_to_steps_vectors(tLinearSpaceGenerators);

    std::ofstream tOutFile(mOutputFileName);
    write_table_header(tSubIndices, tOutFile);

    auto tCombinationGenerator = CombinationGenerator{tStepsVector, SubIndexVector{tSubIndices}, tInitialGuess};
    std::for_each(tCombinationGenerator.begin(), tCombinationGenerator.end(),
                  [&tObjective, &tOutFile, this](const auto aCombination)
                  {
                      const auto tValue = tObjective.evaluate<core::evaluation::kFunction>(
                          linear_algebra::DynamicVector<double>{aCombination});
                      const auto tSubDomain = down_select_to_sub_entries(aCombination, mParameters.first);
                      write_table_row(tSubDomain, tValue, tOutFile);
                  });
}

namespace detail
{
auto validate_output_file_name(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::parameter_sweep>(), aInput.output_file_name, "output_file_name");
}

auto validate_indices_and_ranges(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::parameter_sweep>(), aInput.indices_and_ranges, "indices_and_ranges");
}

namespace
{
const auto kParameterSweepString = input_parser::block_name<input_parser::parameter_sweep>();
const auto kNegativeIndexCondition = [](const auto aIndexAndRange) -> bool { return aIndexAndRange.index < 0; };
const auto kBadLimits = [](const auto aIndexAndRange) -> bool
{ return aIndexAndRange.range.mLower > aIndexAndRange.range.mUpper; };
const auto kNegativeStep = [](const auto aIndexAndRange) -> bool { return aIndexAndRange.range.mStep <= 0; };

template <typename Function>
concept CheckFunction = requires(Function aFunction) {
    { aFunction(input_parser::IndexAndRange{}) } -> std::convertible_to<bool>;
};
template <CheckFunction CheckFunction>
[[nodiscard]] auto check_condition(const std::vector<input_parser::IndexAndRange>& aIndicesAndRanges,
                                   std::vector<std::string> aMessages,
                                   const std::string& aInfoString,
                                   CheckFunction aFunction) -> std::vector<std::string>
{
    auto tMessages = std::move(aMessages);
    utilities::transform_if(
        aIndicesAndRanges, std::back_inserter(tMessages), [aInfoString](const auto aIndexAndRange)
        { return kParameterSweepString + aInfoString + std::to_string(aIndexAndRange.index); }, aFunction);
    return tMessages;
}

template <CheckFunction CheckFunction>
[[nodiscard]] auto check_index_and_range_for_condition(const input_parser::parameter_sweep& aInput,
                                                       const std::string& aInfoString,
                                                       CheckFunction aFunction) -> std::optional<std::string>
{
    if (aInput.indices_and_ranges)
    {
        const auto tIndicesAndRanges = aInput.indices_and_ranges.value().mList;
        std::vector<std::string> tMessages;
        tMessages.reserve(tIndicesAndRanges.size());
        tMessages = check_condition(tIndicesAndRanges, tMessages, aInfoString, aFunction);
        if (!tMessages.empty())
        {
            return utilities::concatenate_container(tMessages, "\n");
        }
    }
    return std::nullopt;
}

}  // namespace

auto validate_control_indices(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>
{
    return check_index_and_range_for_condition(aInput, ": control index must be positive, found index ",
                                               kNegativeIndexCondition);
}

auto validate_range_limits(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>
{
    return check_index_and_range_for_condition(aInput, ": starting value must be less than the ending value for index ",
                                               kBadLimits);
}

auto validate_range_steps(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>
{
    return check_index_and_range_for_condition(aInput, ": negative step size found for index ", kNegativeStep);
}

auto validate_control_indices_no_duplicates(const input_parser::parameter_sweep& aInput) -> std::optional<std::string>
{
    if (aInput.indices_and_ranges)
    {
        const auto tIndicesAndRanges = aInput.indices_and_ranges.value().mList;
        std::set<std::size_t> tIndices;
        std::ranges::transform(tIndicesAndRanges, std::inserter(tIndices, tIndices.end()),
                               [](const auto aIndexAndRange) { return aIndexAndRange.index; });

        if (tIndices.size() < tIndicesAndRanges.size())
        {
            return kParameterSweepString + ": duplicate control index found.";
        }
    }
    return std::nullopt;
}

void output_table(const std::filesystem::path& aPath, const Domain& aDomain, const Values& aValues)
{
    assert(aDomain.mValue.size() == aValues.mValue.size());
    std::ofstream tOutFile(aPath);
    tOutFile << std::setw(kWidth) << "Domain" << std::setw(kWidth + 1) << "Value" << std::endl;
    for (const auto& [tDomain, tValue] : utilities::Zip(aDomain.mValue, aValues.mValue))
    {
        tOutFile << FixedFormatNumber{tDomain} << "," << FixedFormatNumber{tValue} << std::endl;
    }
}

auto extract_parameters(const std::vector<input_parser::IndexAndRange>& aIndexAndRanges)
    -> std::pair<std::vector<std::size_t>, std::vector<utilities::LinearSpaceGenerator>>
{
    std::vector<std::size_t> tIndices;
    tIndices.reserve(aIndexAndRanges.size());
    std::vector<utilities::LinearSpaceGenerator> tLinearSpaceGenerators;
    tLinearSpaceGenerators.reserve(aIndexAndRanges.size());
    std::for_each(aIndexAndRanges.begin(), aIndexAndRanges.end(),
                  [&tIndices, &tLinearSpaceGenerators](const auto& aIndexAndRange)
                  {
                      tIndices.push_back(aIndexAndRange.index);
                      tLinearSpaceGenerators.push_back(utilities::LinearSpaceGenerator{
                          aIndexAndRange.range.mLower, aIndexAndRange.range.mStep, aIndexAndRange.range.mUpper});
                  });

    return std::make_pair(tIndices, tLinearSpaceGenerators);
}

}  // namespace detail

}  // namespace plato::process_manager::extension
