#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();

struct DensityTopologyValidationFileFixture : public test_utilities::FileCreatingTestFixture
{
    DensityTopologyValidationFileFixture() : FileCreatingTestFixture{kDensityTopology.mesh_name.value().mToken} {}
};

}  // namespace

TEST(DensityTopologyValidation, ValidateMeshName)
{
    auto tDensityTopology = kDensityTopology;
    EXPECT_FALSE(library::detail::validate_mesh_name(tDensityTopology).has_value());
    tDensityTopology.mesh_name = boost::none;
    EXPECT_TRUE(library::detail::validate_mesh_name(tDensityTopology).has_value());
}

TEST(DensityTopologyValidation, ValidateOutputName)
{
    auto tDensityTopology = kDensityTopology;
    EXPECT_FALSE(detail::validate_output_name(tDensityTopology).has_value());
    tDensityTopology.output_name = boost::none;
    EXPECT_TRUE(detail::validate_output_name(tDensityTopology).has_value());
}

TEST_F(DensityTopologyValidationFileFixture, ValidDensityTopologyInput)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mDensityTopology = kDensityTopology;

    std::vector<std::string> tMessages;
    tMessages = library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_TRUE(tMessages.empty());
}
}  // namespace plato::geometry::extension::unittest