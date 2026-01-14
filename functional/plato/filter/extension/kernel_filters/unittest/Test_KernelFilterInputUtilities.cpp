#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <fstream>

#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReflectFilter.hpp"
#include "plato/filter/extension/kernel_filters/RevolveFilter.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{

constexpr auto kFilterRadius = 0.5;
const auto kRevolveInput = test_utilities::make_filter_input<input_parser::y_axis_revolve_filter>(
    kFilterRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered);

}  // namespace

TEST(RevolveFilterDetail, ValidateTargetMeshExists)
{
    {
        const auto tErrorMessage = validate_target_mesh_exists<input_parser::y_axis_revolve_filter>(kRevolveInput);
        EXPECT_TRUE(tErrorMessage.has_value()) << " No mesh on disk.";
    }
    {
        [[maybe_unused]] const auto tOutputFile = std::ofstream(kRevolveInput.target_mesh_name.value().mToken);
        const auto tErrorMessage = validate_target_mesh_exists<input_parser::y_axis_revolve_filter>(kRevolveInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << " Mesh on disk.";
        std::filesystem::remove(kRevolveInput.target_mesh_name.value().mToken);
    }
}

TEST(KernelFilterInputUtilitiesDetail, SearchResultsContainAllRowIDs)
{
    namespace tpis = third_party_integration::stk_search;
    const auto tSize = std::size_t{3};
    const auto tSearchResultsMissingID2 = std::vector<std::pair<tpis::Identifier, tpis::Identifier>>{
        {{0, 0}, {1, 2}}, {{0, 1}, {3, 4}}, {{1, 0}, {5, 6}}, {{1, 0}, {7, 8}}, {{0, 10}, {9, 10}},
    };
    const auto tSearchResultsAllID = std::vector<std::pair<tpis::Identifier, tpis::Identifier>>{
        {{2, 0}, {1, 2}}, {{1, 1}, {3, 4}}, {{0, 0}, {5, 6}}, {{1, 0}, {7, 8}}, {{0, 10}, {9, 10}},
    };
    {
        const auto tAllFound = detail::search_results_contain_all_row_ids(tSearchResultsMissingID2, tSize);
        EXPECT_FALSE(tAllFound);
    }
    {
        const auto tAllFound = detail::search_results_contain_all_row_ids(tSearchResultsAllID, tSize);
        EXPECT_TRUE(tAllFound);
    }
}

namespace
{
const auto kSourceMeshFile = std::filesystem::path{"source.exo"};

void create_mesh(const std::filesystem::path& aMeshFile,
                 const third_party_integration::stk_io::CommandBounds& aLowerBounds)
{
    namespace tpis = third_party_integration::stk_io;
    const auto tCommandGenerator = tpis::CommandGenerator{.mElements = {10, 10, 10},
                                                          .mLowerBounds = aLowerBounds,
                                                          .mUpperBounds = {1, 1, 1},
                                                          .mType = tpis::CommandElementType::Hex};
    tpis::write_mesh(aMeshFile, tCommandGenerator.toString());
}

[[nodiscard]] auto run_validation_test(const third_party_integration::stk_io::CommandBounds& aLowerBounds,
                                       const input_parser::KernelFilterCenteringTypes& aCentering)
    -> std::optional<std::string>
{
    const auto tReflectInput =
        test_utilities::make_filter_input<input_parser::positive_octant_reflect_filter>(1.5, aCentering);

    create_mesh(kSourceMeshFile, {0, 0, 0});
    create_mesh(tReflectInput.target_mesh_name.value().mToken, aLowerBounds);

    const auto tErrorMessage = validate_all_target_domain_find_source_domain(
        tReflectInput, kSourceMeshFile,
        [](const input_parser::positive_octant_reflect_filter& aInput,
           const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> ReflectFilterType
        { return make_positive_octant_reflect_filter_type(aInput, aAnalysisDomainMesh); },
        boost::mpi::communicator{});

    std::filesystem::remove(kSourceMeshFile);
    std::filesystem::remove(tReflectInput.target_mesh_name.value().mToken);

    return tErrorMessage;
}

void expect_valid_source_and_target(const third_party_integration::stk_io::CommandBounds& aLowerBounds,
                                    const input_parser::KernelFilterCenteringTypes& aCentering,
                                    const plato::test_utilities::TestContext& aTestContext)
{
    const auto tErrorMessage = run_validation_test(aLowerBounds, aCentering);
    EXPECT_FALSE(tErrorMessage.has_value()) << aTestContext;
}

void expect_invalid_source_and_target(const third_party_integration::stk_io::CommandBounds& aLowerBounds,
                                      const input_parser::KernelFilterCenteringTypes& aCentering,
                                      const plato::test_utilities::TestContext& aTestContext)
{
    const auto tErrorMessage = run_validation_test(aLowerBounds, aCentering);
    EXPECT_TRUE(tErrorMessage.has_value()) << aTestContext;
}

}  // namespace

TEST(KernelFilterInputUtilities, ValidateAllTargetDomainFindSourceDomain)
{
    expect_valid_source_and_target({-1, -1, -1}, input_parser::KernelFilterCenteringTypes::kNodeCentered,
                                   TEST_CONTEXT("Target nodal domain should find all source entries."));
    expect_valid_source_and_target({-1, -1, -1}, input_parser::KernelFilterCenteringTypes::kElementCentered,
                                   TEST_CONTEXT("Target elemental domain should find all source entries."));
    expect_invalid_source_and_target({-6, -7, -5}, input_parser::KernelFilterCenteringTypes::kNodeCentered,
                                     TEST_CONTEXT("Target nodal domain should not find all source entries."));
    expect_invalid_source_and_target({-6, -7, -5}, input_parser::KernelFilterCenteringTypes::kElementCentered,
                                     TEST_CONTEXT("Target elemental domain should not find all source entries."));
}

}  // namespace plato::filter::extension::kernel_filters::unittest
