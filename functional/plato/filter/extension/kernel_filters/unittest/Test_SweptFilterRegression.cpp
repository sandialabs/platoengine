#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/SweptFilter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

namespace
{
const auto kSquareSourceMeshFile = utilities::data_file_path("square_2d.cdf");
const auto kBarTargetMeshFile = std::filesystem::path{"bar_3d.exo"};
const auto kZHat = third_party_integration::common::Vector3{0, 0, 1};
void create_bar_mesh()
{
    namespace tpis = third_party_integration::stk_io;
    const tpis::CommandBounds tLowerBounds{-5, -5, 0};
    const tpis::CommandBounds tUpperBounds{5, 5, 5};
    const tpis::CommandNumberOfElements tElements{10, 10, 5};
    const auto tCommunicatorandGenerator =
        tpis::CommandGenerator{tElements, tLowerBounds, tUpperBounds, tpis::CommandElementType::Hex};
    tpis::write_mesh(kBarTargetMeshFile, tCommunicatorandGenerator.toString());
}
auto create_valid_symmetry_filter() -> input_parser::z_swept_filter
{
    return input_parser::z_swept_filter{/*.filter_radius=*/17.0,
                                        /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kNodeCentered,
                                        /*.use_relative_radius=*/boost::none,
                                        /*.target_mesh_name=*/
                                        input_parser::FileName{kBarTargetMeshFile},
                                        /*.number_of_processors*/ 1,
                                        /*.fixed_blocks=*/boost::none};
}

}  // namespace

TEST(SweptFilter, FilterRegression)
{
    ASSERT_TRUE(kSquareSourceMeshFile.has_value());
    create_bar_mesh();
    const auto tSourceMesh = mesh::Mesh{kSquareSourceMeshFile.value()};
    const auto tTargetMesh = mesh::Mesh{kBarTargetMeshFile};
    const auto tRadius = 3.0;
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tFilterType =
        detail::make_z_swept_filter_type(tRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered, kZHat);
    const auto tFilter = KernelFilter<input_parser::z_swept_filter>{SourceMesh{tSourceMesh}, TargetMesh{tTargetMesh},
                                                                    tFilterType, tCommunicator};
    auto tUnfiltered = std::vector<double>(mesh::EntityCounts{tSourceMesh}.numberOfDesignDomainNodes(), 0.0);
    tUnfiltered[26] = 1;

    const auto tSourceAnalysisDomainMesh = mesh::DesignVariablesConversion{tSourceMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tUnfiltered});
    const auto tFilteredAnalysisDomainMesh = tFilter.filter(tSourceAnalysisDomainMesh);
    EXPECT_EQ(tFilteredAnalysisDomainMesh.mFileName, kBarTargetMeshFile);
    const auto tFilteredResult = mesh::DesignVariablesConversion{tTargetMesh}
                                     .analysisDomainMeshToNodalFieldVector(tFilteredAnalysisDomainMesh)
                                     .mValue;
    ASSERT_EQ(tFilteredResult.size(), mesh::EntityCounts{tTargetMesh}.numberOfDesignDomainNodes());
    ASSERT_EQ(tFilteredResult.size(), 726U);

    EXPECT_DOUBLE_EQ(tFilteredResult.at(0), 0);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(1), 0);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(2), 0);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(3), 0);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(335), 0.1082711823295502);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(336), 0.1301017532145269);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(337), 0.1082711823295502);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(554), 0.1301017532145269);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(555), 0.2834576353408995);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(699), 0.1301017532145269);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(700), 0.1082711823295502);
    EXPECT_DOUBLE_EQ(tFilteredResult.at(701), 0.02232193187457794);
    std::filesystem::remove(kBarTargetMeshFile);
}

TEST(SweptFilterDetail, CreateFilterCache)
{
    ASSERT_TRUE(kSquareSourceMeshFile.has_value());
    create_bar_mesh();
    const auto tSourceMesh = mesh::Mesh{kSquareSourceMeshFile.value()};

    auto tInput = create_valid_symmetry_filter();
    tInput.target_mesh_name = input_parser::FileName{kBarTargetMeshFile};
    auto tFilterCache = detail::create_kernel_filter_cache<input_parser::z_swept_filter>(
        tInput,
        [](const input_parser::z_swept_filter& aInput,
           const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> SweptFilterType
        { return make_z_swept_filter_type(aInput, aAnalysisDomainMesh); });
    const auto tNodalDensities = std::vector<double>(mesh::EntityCounts{tSourceMesh}.numberOfNodes(), 1.0);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tSourceMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalDensities});

    ASSERT_NO_THROW([[maybe_unused]] const auto tFilter = tFilterCache.compute(tAnalysisDomainMesh));
    const auto tFilteredAnalysisDomainMesh = tFilterCache.compute(tAnalysisDomainMesh)->filter(tAnalysisDomainMesh);

    const auto tTargetMesh = mesh::Mesh{kBarTargetMeshFile};
    const auto tResult = mesh::DesignVariablesConversion{tTargetMesh}
                             .analysisDomainMeshToNodalFieldVector(tFilteredAnalysisDomainMesh)
                             .mValue;

    const auto tGoldSize = mesh::EntityCounts{tTargetMesh}.numberOfNodes();
    EXPECT_EQ(tResult.size(), tGoldSize);
    std::filesystem::remove(kBarTargetMeshFile);
}

TEST(SweptFilterDetail, CreateLinearMask)
{
    // Three nodes (0,0,0), (1e-1,1,0), (0,2,0) [increasing in y, second coordinate with delta x for positive area]
    const auto tSourceMeshFile = utilities::data_file_path("three_node_2d.cdf");
    // One node (0,0,2)
    const auto tTargetMeshFile = utilities::data_file_path("one_node_3d.cdf");

    ASSERT_TRUE(tSourceMeshFile.has_value());
    ASSERT_TRUE(tTargetMeshFile.has_value());

    const auto tSourceMesh = mesh::Mesh{tSourceMeshFile.value()};
    const auto tTargetMesh = mesh::Mesh{tTargetMeshFile.value()};
    const auto tRadius = 3.0;
    const auto tFilterType =
        detail::make_z_swept_filter_type(tRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered, kZHat);

    const auto tLinearMask = detail::create_linear_mask<input_parser::z_swept_filter>(
        SourceMesh{tSourceMesh}, TargetMesh{tTargetMesh}, tFilterType, boost::mpi::communicator{});
    const auto tFieldValues = std::vector{10.0, 7.5, 5.0};
    const auto tFilteredValues = tLinearMask.matrixMultiply(tFieldValues);
    ASSERT_EQ(tFilteredValues.size(), 1U);
    const auto tGold = 8.3340266266005507;
    EXPECT_DOUBLE_EQ(tFilteredValues[0], tGold);
}

}  // namespace plato::filter::extension::kernel_filters::unittest
