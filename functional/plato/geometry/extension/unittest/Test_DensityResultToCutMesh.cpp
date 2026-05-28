#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <boost/none.hpp>
#include <filesystem>
#include <set>
#include <vector>

#include "plato/geometry/extension/DensityResultToCutMesh.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
const auto kOutputFile = std::string{"mesh.exo"};
const auto kOutputFileOnDisk = "cut_" + kOutputFile;
const auto kFixedBlock = std::string{"block_1"};

const auto kDensityInput =
    input_parser::density_topology{/*.mesh_name=*/boost::none,
                                   /*.output_name=*/input_parser::FileName{kOutputFile},
                                   /*.fixed_blocks=*/input_parser::BlockList{{kFixedBlock}},
                                   /*.filter=*/boost::none,
                                   /*.initial_density_value=*/boost::none,
                                   /*.initial_density_field_name=*/boost::none,
                                   /*.cut_mesh_file_name=*/input_parser::FileName{kOutputFileOnDisk}};

[[nodiscard]] auto create_nodal_centered_analysis_domain_mesh(const std::vector<double>& aDensities)
    -> analysis::AnalysisDomainMesh
{
    EXPECT_EQ(aDensities.size(), 4U);
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value(), std::set<std::string>{kFixedBlock}};
    return mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aDensities});
}

[[nodiscard]] auto create_element_centered_analysis_domain_mesh() -> analysis::AnalysisDomainMesh
{
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value(), std::set<std::string>{kFixedBlock}};
    const auto tDensities = std::vector<double>{0.0, 1.0};
    return mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{tDensities});
}

}  // namespace

TEST(DensityResultToCutMesh, WriteDensityResultAsCutMeshNodeCentered)
{
    const auto tDensities = std::vector<double>{0.0, 1.0 / 3.0, 2.0 / 3.0, 1.0};
    write_density_result_as_cut_mesh(create_nodal_centered_analysis_domain_mesh(tDensities), kDensityInput,
                                     boost::mpi::communicator{});
    test_utilities::test_for_existence_and_remove({kOutputFileOnDisk}, TEST_CONTEXT("Node centered cut mesh exists"));
}

TEST(DensityResultToCutMesh, WriteDensityResultAsCutMeshNodeCenteredNoMeshWritten)
{
    const auto tDensities = std::vector<double>{0.0, 0.0, 0.0, 0.0};
    auto tDensityInput = kDensityInput;
    tDensityInput.fixed_blocks = boost::none;
    write_density_result_as_cut_mesh(create_nodal_centered_analysis_domain_mesh(tDensities), tDensityInput,
                                     boost::mpi::communicator{});
    EXPECT_FALSE(std::filesystem::exists(kOutputFileOnDisk)) << "No zero crossing, no mesh written.";
}

TEST(DensityResultToCutMesh, ZeroCrossingExists)
{
    {
        const auto tLevelSetValues = std::vector<double>{1.0, 1.0, 1.0, 1.0};
        EXPECT_FALSE(detail::zero_crossing_exists(tLevelSetValues)) << "All ones";
    }

    {
        const auto tLevelSetValues = std::vector<double>{0.0, 0.0, 1.0, 1.0};
        EXPECT_TRUE(detail::zero_crossing_exists(tLevelSetValues)) << "Zeros and ones";
    }
    {
        const auto tLevelSetValues = std::vector<double>{0.0, 0.0, 0.0, 0.0};
        EXPECT_FALSE(detail::zero_crossing_exists(tLevelSetValues)) << "All zeros";
    }
    {
        const auto tLevelSetValues = std::vector<double>{-1.0, -1.0, 0.0, 0.0};
        EXPECT_FALSE(detail::zero_crossing_exists(tLevelSetValues)) << "Negative ones and zeros";
    }
    {
        const auto tLevelSetValues = std::vector<double>{-1.0, -1.0, 1.0, 1.0};
        EXPECT_TRUE(detail::zero_crossing_exists(tLevelSetValues)) << "Negative ones and ones";
    }
}

TEST(DensityResultToCutMesh, WriteDensityResultAsCutMeshElementCentered)
{
    write_density_result_as_cut_mesh(create_element_centered_analysis_domain_mesh(), kDensityInput,
                                     boost::mpi::communicator{});
    test_utilities::test_for_existence_and_remove({kOutputFileOnDisk},
                                                  TEST_CONTEXT("Element centered cut mesh exists"));
}

TEST(DensityResultToCutMesh, ConvertToNodal)
{
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value(), std::set<std::string>{kFixedBlock}};
    const auto tAnalysisDomainMesh = create_element_centered_analysis_domain_mesh();
    const auto tNodalAnalysisDomainMesh = detail::convert_to_nodal(tAnalysisDomainMesh, tMesh);
    const auto tNodalValues =
        mesh::DesignVariablesConversion{tMesh}.analysisDomainMeshToNodalFieldVector(tNodalAnalysisDomainMesh).mValue;
    const auto tGoldValues = std::vector<double>{0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
    EXPECT_EQ(tNodalValues, tGoldValues);
}

TEST(DensityResultToCutMesh, TransformToLevelSetField)
{
    constexpr auto tThreshold = 0.5;
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value(), std::set<std::string>{kFixedBlock}};
    const auto tDensities = std::vector<double>{0.0, 1.0 / 3.0, 2.0 / 3.0, 1.0};
    const auto tAnalysisDomainMesh = create_nodal_centered_analysis_domain_mesh(tDensities);
    const auto tShiftedAnalysisDomainMesh = detail::transform_to_level_set_field(tAnalysisDomainMesh, tThreshold);

    const auto tOriginalValues =
        mesh::DesignVariablesConversion{tMesh}.analysisDomainMeshToNodalFieldVector(tAnalysisDomainMesh).mValue;
    const auto tShiftedValues =
        mesh::DesignVariablesConversion{tMesh}.analysisDomainMeshToNodalFieldVector(tShiftedAnalysisDomainMesh).mValue;

    for (const auto& [tOriginal, tShifted] : utilities::Zip{tOriginalValues, tShiftedValues})
    {
        EXPECT_EQ(tOriginal - tThreshold, tShifted);
    }
}

}  // namespace plato::geometry::extension::unittest
