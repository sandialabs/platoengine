#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "plato/criteria/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/input_parser/Range.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/IndexAndRange.hpp"
#include "plato/process_manager/extension/ParameterSweep.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{

namespace
{
const auto kExampleIndexAndRangeOne = input_parser::IndexAndRange{0U, input_parser::Range{0, 1, 4}};
const auto kExampleIndexAndRangeTwo = input_parser::IndexAndRange{1U, input_parser::Range{-4, 1, 0}};
const auto kExampleIndexAndRangeThree = input_parser::IndexAndRange{2U, input_parser::Range{0, 1, 1}};

[[nodiscard]] auto create_valid_example_parameter_sweep() -> input_parser::parameter_sweep
{
    return input_parser::parameter_sweep{
        /*.output_file_name=*/input_parser::FileName{"parameter_sweep.txt"},
        /*.indices_and_ranges=*/input_parser::IndexAndRangeList{{kExampleIndexAndRangeOne, kExampleIndexAndRangeTwo}}};
}

void check_table_rows(const std::filesystem::path& aFileName,
                      const unsigned int aGoldRowCount,
                      const plato::test_utilities::TestContext& aTestContext)
{
    std::size_t tRowCount = 0;
    std::string tLine;
    std::ifstream tInFile(aFileName);
    while (std::getline(tInFile, tLine))
    {
        ++tRowCount;
    }
    EXPECT_EQ(tRowCount, aGoldRowCount) << aTestContext;
}

void test_run_parameter_sweep(const input_parser::parameter_sweep& aParameterSweepInput,
                              const unsigned int aGoldRowCount,
                              const plato::test_utilities::TestContext& aTestContext)
{
    const auto tInputDeck = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            criteria::extension::test_utilities::create_valid_example_nodal_sum_objective_input() |
                            aParameterSweepInput;

    const auto tValidatedInput = input_validation::make_validated_input(tInputDeck);

    const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput.value());

    const auto tAllProcessManagerInputs = tValidatedInput.value().get<components::ComponentType::kProcessManager>();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1u);
    const auto tParameterSweep = ParameterSweep{tAllProcessManagerInputs.rawInput().back()};

    tParameterSweep.run(tProblem);
    check_table_rows(aParameterSweepInput.output_file_name.value().mToken, aGoldRowCount, aTestContext);
    plato::test_utilities::test_for_existence_and_remove({aParameterSweepInput.output_file_name.value().mToken},
                                                         aTestContext);
}

}  // namespace

TEST(ParameterSweep, CreateParameterSweepRunOneVariable)
{
    auto tParameterSweepInput = create_valid_example_parameter_sweep();
    tParameterSweepInput.indices_and_ranges.value().mList.pop_back();
    constexpr auto tGoldRowCount = 6U;
    test_run_parameter_sweep(tParameterSweepInput, tGoldRowCount, TEST_CONTEXT("One variable parameter sweep."));
}

TEST(ParameterSweep, CreateParameterSweepRunTwoVariables)
{
    const auto tParameterSweepInput = create_valid_example_parameter_sweep();
    constexpr auto tGoldRowCount = 26U;
    test_run_parameter_sweep(tParameterSweepInput, tGoldRowCount, TEST_CONTEXT("Two variable parameter sweep."));
}

TEST(ParameterSweep, CreateParameterSweepRunThreeVariables)
{
    auto tParameterSweepInput = create_valid_example_parameter_sweep();
    tParameterSweepInput.indices_and_ranges.value().mList.push_back(kExampleIndexAndRangeThree);
    constexpr auto tGoldRowCount = 51U;
    test_run_parameter_sweep(tParameterSweepInput, tGoldRowCount, TEST_CONTEXT("Three variable parameter sweep."));
}

TEST(ParameterSweepDetail, ValidateOuputFileName)
{
    auto tParameterSweep = input_parser::parameter_sweep{};
    EXPECT_TRUE(detail::validate_output_file_name(tParameterSweep).has_value())
        << "Invalid - requires output file name.";
    tParameterSweep.output_file_name = input_parser::FileName{"file.txt"};
    EXPECT_FALSE(detail::validate_output_file_name(tParameterSweep).has_value()) << "Valid output file name.";
}

TEST(ParameterSweepDetail, ValidateIndicesAndRanges)
{
    input_parser::parameter_sweep tParameterSweep;
    EXPECT_TRUE(detail::validate_indices_and_ranges(tParameterSweep).has_value()) << "Required input.";

    tParameterSweep.indices_and_ranges =
        input_parser::IndexAndRangeList{{input_parser::IndexAndRange{1U, input_parser::Range{0.0, 10.0, 100.0}}}};
    EXPECT_FALSE(detail::validate_indices_and_ranges(tParameterSweep).has_value()) << "Valid range.";
}

TEST(ParameterSweepDetail, ValidateControlIndices)
{
    auto tParameterSweep = create_valid_example_parameter_sweep();
    EXPECT_FALSE(detail::validate_control_indices(tParameterSweep).has_value()) << "Valid control indices.";
    tParameterSweep.indices_and_ranges.value().mList.push_back(
        input_parser::IndexAndRange{-1, input_parser::Range{0.0, 10.0, 100.0}});
    EXPECT_TRUE(detail::validate_control_indices(tParameterSweep).has_value()) << "Invalid control index.";
}

TEST(ParameterSweepDetail, ValidateControlIndicesNoDuplicates)
{
    auto tParameterSweep = create_valid_example_parameter_sweep();
    EXPECT_FALSE(detail::validate_control_indices_no_duplicates(tParameterSweep).has_value())
        << "Valid control indices.";
    tParameterSweep.indices_and_ranges.value().mList.push_back(
        input_parser::IndexAndRange{1U, input_parser::Range{0.0, 10.0, 100.0}});
    EXPECT_TRUE(detail::validate_control_indices_no_duplicates(tParameterSweep).has_value())
        << "Invalid control indices, duplicate found.";
}

TEST(ParameterSweepDetail, ValidateRangeLimits)
{
    auto tParameterSweep = create_valid_example_parameter_sweep();
    EXPECT_FALSE(detail::validate_range_limits(tParameterSweep).has_value()) << "Valid range limits.";
    tParameterSweep.indices_and_ranges.value().mList = {
        input_parser::IndexAndRange{1U, input_parser::Range{100.0, 10.0, 0.0}}};
    EXPECT_TRUE(detail::validate_range_limits(tParameterSweep).has_value())
        << "Invalid range, start is larger than end.";
}

TEST(ParameterSweepDetail, ValidateRangeSteps)
{
    auto tParameterSweep = create_valid_example_parameter_sweep();
    EXPECT_FALSE(detail::validate_range_steps(tParameterSweep).has_value()) << "Valid range limits.";

    tParameterSweep.indices_and_ranges.value().mList.front() =
        input_parser::IndexAndRange{1U, input_parser::Range{0.0, -10.0, 100.0}};
    EXPECT_TRUE(detail::validate_range_steps(tParameterSweep).has_value()) << "Invalid range, negative step size.";
}

TEST(ParameterSweepDetail, OutputTable)
{
    const auto tDomain = std::vector<double>{0, 1, 2, 3};
    const auto tValues = std::vector<double>{4, 3, 2, 4};
    const auto tFileName = std::filesystem::path{"Table.txt"};

    detail::output_table(tFileName, Domain{tDomain}, Values{tValues});

    plato::test_utilities::test_for_existence_and_remove({tFileName}, TEST_CONTEXT("Removing table from disk."));
}

TEST(ParameterSweepDetail, ExtractParameters)
{
    const auto tResult = detail::extract_parameters({kExampleIndexAndRangeOne, kExampleIndexAndRangeTwo});

    ASSERT_EQ(tResult.first.size(), 2U);
    EXPECT_EQ(tResult.first.front(), kExampleIndexAndRangeOne.index);
    EXPECT_EQ(tResult.first.back(), kExampleIndexAndRangeTwo.index);
    EXPECT_EQ(tResult.second.front().mStartingValue, kExampleIndexAndRangeOne.range.mLower);
    EXPECT_EQ(tResult.second.front().mStep, kExampleIndexAndRangeOne.range.mStep);
    EXPECT_EQ(tResult.second.back().mEndingValue, kExampleIndexAndRangeTwo.range.mUpper);
}

}  // namespace plato::process_manager::extension::unittest
