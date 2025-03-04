#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string_view>

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/test_utilities/FilterFunction.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::geometry::extension::parallel_unittest
{
using NodalDensityMesh = third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;

namespace
{
[[nodiscard]] auto density_input(const std::filesystem::path& aMeshName, const std::string_view aFieldName)
    -> input_parser::density_topology
{
    auto tDensityInput = input_parser::density_topology{};
    tDensityInput.mesh_name = input_parser::FileName{aMeshName};
    tDensityInput.output_name = input_parser::FileName{"test_out.exo"};
    tDensityInput.initial_density_field_name = input_parser::IdentifierString{std::string{aFieldName}};
    return tDensityInput;
}
}  // namespace

TEST_F(NodalDensityMesh, ParallelOutput)
{
    namespace fe = filter::extension;

    const auto tDensityInput = density_input(mMeshName, mFieldName);
    const auto tDesignVariables = DensityTopology::initialGuess(tDensityInput);
    const auto tFilter = filter::test_utilities::make_filter_function(std::make_shared<fe::KernelFilter>(
        mesh::Mesh{mMeshName}, fe::FilterRadius{3.25}, input_parser::KernelFilterCenteringTypes::kElementCentered,
        boost::mpi::communicator{}));
    DensityTopology::output(tDesignVariables, tFilter, tDensityInput, library::kOverwriteInfo);
    constexpr auto kAppend = false;
    DensityTopology::output(tDesignVariables, tFilter, tDensityInput, library::OutputInfo{kAppend, 2});

    ASSERT_TRUE(std::filesystem::exists(tDensityInput.output_name.value().mToken));

    for (const auto tTimeStep : {1.0, 2.0})
    {
        // Control densities
        constexpr auto tDensityFieldName = density_mesh_field_name();
        const auto tReadUnfilteredDensities =
            third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(
                tDensityInput.output_name.value().mToken, tDensityFieldName, tTimeStep);
        EXPECT_EQ(tDesignVariables.stdVector(), tReadUnfilteredDensities);
        // Filtered densities
        constexpr auto tFilteredDensityFieldName = filtered_density_mesh_field_name();
        const auto tReadFilteredDensities =
            third_party_integration::stk_io::test_utilities::read_element_field_as_vector(
                tDensityInput.output_name.value().mToken, tFilteredDensityFieldName, tTimeStep);
        const auto tExpected = std::vector<double>{8.435398473291716, 8.5, 8.564601526708284};  // Regression value
        constexpr auto tTolerance = 1e-14;
        test_utilities::expect_container_entries_near(tExpected, tReadFilteredDensities, tTolerance,
                                                      TEST_CONTEXT("Filtered density values"));
    }
    std::filesystem::remove(tDensityInput.output_name->mToken);
}

}  // namespace plato::geometry::extension::parallel_unittest
