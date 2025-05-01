#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <numeric>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::parallel_unittest
{
using third_party_integration::krino::test_utilities::KrinoTestFixture;
namespace
{
namespace tpik = third_party_integration::krino;
const auto kBoxFilePath = utilities::data_file_path("box_3x4x7_tet4.cdf");
const auto kUnitSphere = tpik::Sphere{{0, 0, 0}, 1};
const auto kThreeQuarterOffsetXHatPlane = tpik::Plane{{-1, 0, 0}, 0.75};
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
const auto kFourTriSensitivityMapRankZero =
    tpik::SensitivityMap{{9, tpik::LevelSetJacobianColumn{{4, 5}, {{0.75, 0, 0}, {0.25, 0, 0}}, {2, 3}}},
                         {10, tpik::LevelSetJacobianColumn{{4, 6}, {{0.75, -0.75, 0}, {0.25, -0.25}}, {2, 4}}},
                         {8, tpik::LevelSetJacobianColumn{{1, 4}, {{.25, 0.25, 0}, {0.75, 0.75, 0}}, {0, 2}}}};

const auto kFourTriSensitivityMapRankOne =
    tpik::SensitivityMap{{13, tpik::LevelSetJacobianColumn{{7, 2}, {{0.25, 0.25, 0}, {0.75, 0.75, 0}}, {5, 1}}},
                         {11, tpik::LevelSetJacobianColumn{{1, 2}, {{0.25, 0.0, 0}, {0.75, 0, 0}}, {0, 1}}},
                         {8, tpik::LevelSetJacobianColumn{{1, 4}, {{.25, 0.25, 0}, {0.75, 0.75, 0}}, {0, 2}}}};

}  // namespace

TEST_F(KrinoTestFixture, KrinoWrapperParallel)
{
    ASSERT_TRUE(kBoxFilePath.has_value());

    const auto tMesh = mesh::Mesh{kBoxFilePath.value()};
    const auto tNumberOfDesignNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
    std::vector<double> tDesignVariable(tNumberOfDesignNodes);
    std::iota(tDesignVariable.begin(), tDesignVariable.end(), -static_cast<double>(tNumberOfDesignNodes) / 2.0);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tDesignVariable});
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);
    const auto tSensitivity = tKrinoWrapper.sensitivities();

    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    if (tRank == 0)
    {
        const auto tGoldSizeRankZero = std::size_t{289};
        EXPECT_EQ(tGoldSizeRankZero, tSensitivity.size()) << "Rank zero sensitivity map size.";
    }
    else
    {
        const auto tGoldSizeRankOne = std::size_t{296};
        EXPECT_EQ(tGoldSizeRankOne, tSensitivity.size()) << "Rank one sensitivity map size.";
    }
}

TEST_F(KrinoTestFixture, FourTriSensitivityMap)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());

    const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
        std::nullopt);
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value()};
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tInitialGuess});
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);

    const auto tSensitivity = tKrinoWrapper.sensitivities();

    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    if (tRank == 0)
    {
        tpik::test_utilities::test_sensitivity_map(tSensitivity, kFourTriSensitivityMapRankZero,
                                                   TEST_CONTEXT("Sensitivity map on Rank Zero."));
    }
    else
    {
        tpik::test_utilities::test_sensitivity_map(tSensitivity, kFourTriSensitivityMapRankOne,
                                                   TEST_CONTEXT("Sensitivity map on Rank One."));
    }
}

TEST_F(KrinoTestFixture, MakeInitialGuessFromLevelSetPrimitives)
{
    ASSERT_TRUE(kBoxFilePath.has_value());
    const auto tMesh = mesh::Mesh{kBoxFilePath.value()};

    {
        const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
            kBoxFilePath.value(), tpik::LevelSetPrimitives{{}, {kUnitSphere}}, std::nullopt);
        const auto tNumberOfDesignNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
        ASSERT_EQ(tInitialGuess.size(), tNumberOfDesignNodes);
    }

    {
        const auto tDesignDomainNodeIds = std::vector<tpik::BackgroundMeshNodeId>{1, 2, 3, 4, 5};
        const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
            kBoxFilePath.value(), tpik::LevelSetPrimitives{{}, {kUnitSphere}}, tDesignDomainNodeIds);
        ASSERT_EQ(tInitialGuess.size(), tDesignDomainNodeIds.size());
    }
}

TEST_F(KrinoTestFixture, RowVectorJacobianProduct)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
        std::nullopt);
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value()};
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tInitialGuess});
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);

    const auto tNumberOfCutMeshNodes = 9;
    const auto tDimensions = 2;
    std::vector<double> tRowVector(tNumberOfCutMeshNodes * tDimensions, 1.0);

    [[maybe_unused]] const auto tResult =
        tKrinoWrapper.rowVectorJacobianProduct(tRowVector, tpik::VoidPhase::kExcludeFromMesh);

    const auto tGold = std::vector<double>{0.75, 2.25, 2.25, 0.25, 0, 0.5};
    EXPECT_EQ(tGold, tResult);
}

TEST_F(KrinoTestFixture, RowVectorAdjointJacobianProduct)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
        std::nullopt);
    const auto tMesh = mesh::Mesh{kFourTriTwoBlockMeshFilePath.value()};
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tInitialGuess});
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);

    const auto tNumberOfBackgroundNodes = 6;
    std::vector<double> tRowVector(tNumberOfBackgroundNodes, 1.0);

    [[maybe_unused]] const auto tResult =
        tKrinoWrapper.rowVectorAdjointJacobianProduct(tRowVector, tpik::VoidPhase::kExcludeFromMesh);

    const auto tGold = std::vector<double>{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, -1, 1, 0, 1, 1};
    EXPECT_EQ(tGold, tResult);
}

TEST_F(KrinoTestFixture, CutMeshNodeIdMultiplicity)
{
    const auto tMesh = tpik::read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField =
        tpik::test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25, 0.75, 0.75, 0.75});
    tpik::cut_mesh(tMesh->bulk_data(), tLevelSetField);
    const auto tDesignDomain = tpik::background_node_ids(*tMesh, tLevelSetField);
    const auto tSensitivityMap = detail::compute_sensitivities(tMesh->bulk_data(), tLevelSetField, tDesignDomain);
    const auto tResult = tpik::cut_mesh_node_id_multiplicity(tSensitivityMap);

    for (const auto& tEntry : tSensitivityMap)
    {
        std::cout << "Sensitivity map cut id: " << tEntry.first << " on rank " << boost::mpi::communicator{}.rank()
                  << std::endl;
    }

    for (const auto& tEntry : tResult)
    {
        std::cout << "Cuti mesh: " << tEntry.first << " with count " << tEntry.second << " on rank "
                  << boost::mpi::communicator{}.rank() << std::endl;
    }

    ASSERT_EQ(tResult.size(), 1U);
    EXPECT_EQ(tResult.at(8U), 2U);
}

}  // namespace plato::geometry::extension::parallel_unittest
