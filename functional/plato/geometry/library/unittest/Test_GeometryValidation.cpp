#include <gtest/gtest.h>

#include <fstream>

#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser),
    test_geometry,
    (plato::input_parser::FileName, mesh_name, ""))

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser),
    another_test_geometry,
    (int, number_of_sides, ""))
// clang-format on

namespace plato::geometry::library::unittest
{
namespace
{
struct GeometryTopologyValidationFileFixture : public test_utilities::FileCreatingTestFixture
{
    GeometryTopologyValidationFileFixture() : FileCreatingTestFixture{"testfile.txt"} {}
};
}  // namespace

TEST(GeometryValidation, InValidParsedInputNoGeometry)
{
    const auto tInput = input_parser::NewParsedInput{};
    EXPECT_TRUE(plato::geometry::library::detail::validate_only_one_geometry(tInput).has_value());
}

TEST(GeometryValidation, ValidParsedInputOneGeometry)
{
    const auto tInput = input_parser::NewParsedInput{} | input_parser::test_geometry{};
    EXPECT_FALSE(plato::geometry::library::detail::validate_only_one_geometry(tInput).has_value());
}

TEST(GeometryValidation, InValidParsedInputTwoGeometry)
{
    const auto tInput = input_parser::test_geometry{} | input_parser::another_test_geometry{};
    EXPECT_TRUE(plato::geometry::library::detail::validate_only_one_geometry(tInput).has_value());
}

TEST_F(GeometryTopologyValidationFileFixture, MeshFileExists)
{
    // Valid
    {
        const auto tGeometryInput = input_parser::test_geometry{input_parser::FileName{filePath().string()}};
        const auto tErrorMessage = detail::validate_mesh_file_exists(tGeometryInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << tErrorMessage.value();
    }
    // Empty
    {
        const auto tGeometryInput = input_parser::test_geometry{boost::none};
        const auto tErrorMessage = detail::validate_mesh_file_exists(tGeometryInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << tErrorMessage.value();
    }
    // Invalid
    {
        const auto tGeometryInput = input_parser::test_geometry{input_parser::FileName{"bogus-file-name.txt"}};
        const auto tErrorMessage = detail::validate_mesh_file_exists(tGeometryInput);
        EXPECT_TRUE(tErrorMessage.has_value());
    }
}

}  // namespace plato::geometry::library::unittest
