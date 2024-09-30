#define BOOST_PHOENIX_STL_TUPLE_H_  // Work-around for ODR violation in boost phoenix.
                                    // https://github.com/boostorg/phoenix/issues/111

#include <gtest/gtest.h>

#include <filesystem>

#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/process_manager/library/CrossLinkedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/IOUtilities.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
void generate_unit_element_mesh(const std::filesystem::path& aMeshFileName)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {2, 2, 2}, {-1, -1, -1}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(aMeshFileName, tCommandGenerator);
}

void test_for_invalid_filter_radius(const input_parser::ParsedInput& aInput,
                                    const test_utilities::TestContext& aTestContext)
{
    const std::filesystem::path tMeshFileName{aInput.mDensityTopology.value().mesh_name.value().mToken};
    generate_unit_element_mesh(tMeshFileName);

    auto tCrossLinkedInput = make_cross_linked_input(aInput);
    EXPECT_TRUE(
        geometry::extension::detail::validate_filter_with_mesh(tCrossLinkedInput.rawInput().mDensityTopology.value())
            .has_value())
        << aTestContext;

    std::filesystem::remove(tMeshFileName);
}

void test_for_valid_filter_radius(const input_parser::ParsedInput& aInput,
                                  const test_utilities::TestContext& aTestContext)
{
    const std::filesystem::path tMeshFileName{aInput.mDensityTopology.value().mesh_name.value().mToken};
    generate_unit_element_mesh(tMeshFileName);

    auto tCrossLinkedInput = make_cross_linked_input(aInput);
    ASSERT_TRUE(tCrossLinkedInput.rawInput().mDensityTopology->filter) << aTestContext;
    ASSERT_TRUE(tCrossLinkedInput.rawInput()
                    .mDensityTopology->filter->mInputBlock.template holds_expected_type<filter::library::FilterInput>())
        << aTestContext;
    EXPECT_FALSE(
        geometry::extension::detail::validate_filter_with_mesh(tCrossLinkedInput.rawInput().mDensityTopology.value())
            .has_value())
        << aTestContext;

    std::filesystem::remove(tMeshFileName);
}
}  // namespace

TEST(ValidateDensityTopologyCrossLinks, NoCrossLinkedFilter)
{
    const auto tInput = test_utilities::create_valid_density_topology_geometry();
    ASSERT_FALSE(tInput.filter);
    EXPECT_FALSE(geometry::extension::detail::validate_filter_with_mesh(tInput).has_value());
}

TEST(ValidateDensityTopologyCrossLinks, CrossLinkIsNotToExpectedType)
{
    auto tInput = test_utilities::create_valid_density_topology_geometry();
    tInput.filter.emplace();
    tInput.filter->mInputBlock = input_parser::CrossReferencedInput{};
    ASSERT_TRUE(tInput.filter);
    EXPECT_FALSE(geometry::extension::detail::validate_filter_with_mesh(tInput).has_value());
}

TEST(ValidateDensityTopologyCrossLinks, HelmholtzFilterRadiusValid)
{
    const auto tInput =
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_helmholtz_filter();

    test_for_valid_filter_radius(tInput, TEST_CONTEXT("Valid filter radius"));
}

TEST(ValidateDensityTopologyCrossLinks, HelmholtzFilterRadiusTooSmall)
{
    constexpr double tFilterRadius = 0.5;
    const std::string tInputString =
        R"(
          begin helmholtz_filter
            filter_radius )" +
        std::to_string(tFilterRadius) + R"(
          end
            )" +
        test_utilities::create_valid_density_topology_geometry_string();

    test_for_invalid_filter_radius(input_parser::parse_input(tInputString), TEST_CONTEXT("Invalid filter radius"));
}

TEST(ValidateDensityTopologyCrossLinks, KernelFilterRadiusTooSmall)
{
    constexpr double tFilterRadius = 0.5;
    const std::string tInputString =
        R"(
          begin kernel_filter
            filter_radius )" +
        std::to_string(tFilterRadius) + R"(
            centering_type element
          end
            )" +
        test_utilities::create_valid_density_topology_geometry_string();

    test_for_invalid_filter_radius(input_parser::parse_input(tInputString), TEST_CONTEXT("Invalid filter radius"));
}

TEST(ValidateDensityTopologyCrossLinks, KernelFilterRelativeRadiusIsValid)
{
    constexpr double tFilterRadius = 0.5;
    const std::string tInputString =
        R"(
          begin kernel_filter
            filter_radius )" +
        std::to_string(tFilterRadius) + R"(
            centering_type element
            use_relative_radius true
          end
            )" +
        test_utilities::create_valid_density_topology_geometry_string();
    const auto tInput = input_parser::parse_input(tInputString);

    test_for_valid_filter_radius(tInput, TEST_CONTEXT("Valid filter radius"));
}
}  // namespace plato::process_manager::library::unittest