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

    // tKrinoWrapper.writeCutMesh("out.exo", tpik::VoidPhase::kExcludeFromMesh);
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
    for (const auto& tSensitivityEntry : tSensitivity)
    {
        const auto tCutMeshId = tSensitivityEntry.first;
        const auto tLevelSetJacobianColumn = tSensitivityEntry.second;
        EXPECT_EQ(tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.size(), 2U);
        std::cout << "Rank: " << tRank << " cut mesh id: " << tCutMeshId << " GIDS ("
                  << tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.front() << ", "
                  << tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.back() << ")  LOCALID: ("
                  << tLevelSetJacobianColumn.mDesignDomainLocalIndex.front() << ", "
                  << tLevelSetJacobianColumn.mDesignDomainLocalIndex.back() << ")  sensitivity: {"
                  << tLevelSetJacobianColumn.mNodalSensitivities.front().x << ", "
                  << tLevelSetJacobianColumn.mNodalSensitivities.front().y << ", "
                  << tLevelSetJacobianColumn.mNodalSensitivities.front().z << "}  {"
                  << tLevelSetJacobianColumn.mNodalSensitivities.back().x << ", "
                  << tLevelSetJacobianColumn.mNodalSensitivities.back().y << ", "
                  << tLevelSetJacobianColumn.mNodalSensitivities.back().z << "} " << std::endl;
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
        std::cout << "full design domain" << std::endl;
        for (const auto& aValue : tInitialGuess)
        {
            std::cout << aValue << std::endl;
        }
    }

    {
        const auto tDesignDomainNodeIds = std::vector<tpik::BackgroundMeshNodeId>{1, 2, 3, 4, 5};
        const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
            kBoxFilePath.value(), tpik::LevelSetPrimitives{{}, {kUnitSphere}}, tDesignDomainNodeIds);
        ASSERT_EQ(tInitialGuess.size(), tDesignDomainNodeIds.size());
        std::cout << "5 design domain" << std::endl;
        for (const auto& aValue : tInitialGuess)
        {
            std::cout << aValue << std::endl;
        }
    }
}

TEST_F(KrinoTestFixture, RowVectorJacobianProduct)
{
    ASSERT_TRUE(kBoxFilePath.has_value());
    const auto tInitialGuess = make_initial_guess_from_level_set_primitives(
        kBoxFilePath.value(), tpik::LevelSetPrimitives{{}, {kUnitSphere}}, std::nullopt);
    const auto tMesh = mesh::Mesh{kBoxFilePath.value()};
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tInitialGuess});
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);

    tKrinoWrapper.writeCutMesh("out.exo", tpik::VoidPhase::kExcludeFromMesh);

    const std::vector<double> tRowVector(504 * 3, 1.0);
    // std::iota(tRowVector.begin(), tRowVector.end(), 1.0);
    [[maybe_unused]] const auto tResult =
        tKrinoWrapper.rowVectorJacobianProduct(tRowVector, tpik::VoidPhase::kExcludeFromMesh);

    /*for (const auto& aEntry : tResult)
    {
        std::cout << aEntry << std::endl;
    }*/
}

}  // namespace plato::geometry::extension::parallel_unittest
