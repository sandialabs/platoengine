#include <gtest/gtest.h>

#include <numeric>
#include <stk_math/StkVector.hpp>
#include <string>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
namespace tpik = third_party_integration::krino;
const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");
const auto kThreeQuarterOffsetXHatPlane = tpik::Plane{{-1, 0, 0}, 0.75};
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
using tpik::test_utilities::KrinoTestFixture;
const auto kOneTriSensitivityMap =
    tpik::SensitivityMap{{6, tpik::LevelSetJacobianColumn{{4, 1}, {{0.75, 0.75, 0}, {0.25, 0.25, 0}}, {2, 0}}},
                         {5, tpik::LevelSetJacobianColumn{{1, 2}, {{.25, 0, 0}, {0.75, 0, 0}}, {0, 1}}}};

const auto kOneTriSensitivityMapNodeFourFixed =
    tpik::SensitivityMap{{6, tpik::LevelSetJacobianColumn{{1}, {{0.25, 0.25, 0}}, {0}}},
                         {5, tpik::LevelSetJacobianColumn{{1, 2}, {{.25, 0, 0}, {0.75, 0, 0}}, {0, 1}}}};

const auto kOneTriSensitivityMapNodeOneAndTwoFixed =
    tpik::SensitivityMap{{6, tpik::LevelSetJacobianColumn{{4}, {{0.75, 0.75, 0}}, {0}}}};

const auto kOneTriSensitivityMapAllNodesFixed = tpik::SensitivityMap{};

const auto kFourTriSensitivityMap =
    tpik::SensitivityMap{{9, tpik::LevelSetJacobianColumn{{4, 1}, {{0.75, 0.75, 0}, {0.25, 0.25, 0}}, {2, 0}}},
                         {12, tpik::LevelSetJacobianColumn{{4}, {{0.75, -0.75, 0}}, {2}}},
                         {10, tpik::LevelSetJacobianColumn{{7, 2}, {{0.25, 0.25, 0}, {0.75, 0.75, 0}}, {3, 1}}},
                         {11, tpik::LevelSetJacobianColumn{{4}, {{0.75, 0.0, 0}}, {2}}},
                         {8, tpik::LevelSetJacobianColumn{{1, 2}, {{.25, 0, 0}, {0.75, 0, 0}}, {0, 1}}}};

}  // namespace

class OneTriMeshKrinoFixture : public tpik::test_utilities::SensitivityTestKrinoFixture
{
   public:
    OneTriMeshKrinoFixture()
        : SensitivityTestKrinoFixture(std::string{kOneTriMeshFilePath.value()}, kOneTriSensitivityMap)
    {
    }
    void SetUp() override
    {
        KrinoTestFixture::SetUp();
        mKrinoMesh = tpik::read_and_setup_for_decomposition(mFileName);
        mLevelSetFields = make_level_set_field_from_primitives(
            tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, mKrinoMesh->bulk_data());
        mDesignDomainBackgroundNodes = tpik::background_node_ids(*mKrinoMesh, mLevelSetFields);
        tpik::cut_mesh(mKrinoMesh->bulk_data(), mLevelSetFields, tpik::SnappingParameters{});
    }

   protected:
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
    std::vector<tpik::BackgroundMeshNodeId> mDesignDomainBackgroundNodes;
};

class FourTriMeshKrinoFixture : public tpik::test_utilities::SensitivityTestKrinoFixture
{
   public:
    FourTriMeshKrinoFixture()
        : SensitivityTestKrinoFixture(std::string{kFourTriTwoBlockMeshFilePath.value()}, kFourTriSensitivityMap)
    {
    }
    void SetUp() override
    {
        KrinoTestFixture::SetUp();
        mKrinoMesh = tpik::read_and_setup_for_decomposition(mFileName);
        mLevelSetFields = make_level_set_field_from_primitives(
            tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}}, mKrinoMesh->bulk_data());
        mDesignDomainBackgroundNodes = std::vector<tpik::BackgroundMeshNodeId>{1U, 2U, 4U, 7U};
        tpik::cut_mesh(mKrinoMesh->bulk_data(), mLevelSetFields, tpik::SnappingParameters{});
    }

   protected:
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
    std::vector<tpik::BackgroundMeshNodeId> mDesignDomainBackgroundNodes;
};

TEST_F(OneTriMeshKrinoFixture, OneTriMesh)
{
    const auto tSensitivities =
        detail::compute_sensitivities(mKrinoMesh->bulk_data(), mLevelSetFields, mDesignDomainBackgroundNodes);
    compareMapAgainstBuiltInGold(tSensitivities, TEST_CONTEXT("OneTriMesh Sensitivities"));
}

TEST_F(FourTriMeshKrinoFixture, FourTriMesh)
{
    const auto tSensitivities =
        detail::compute_sensitivities(mKrinoMesh->bulk_data(), mLevelSetFields, mDesignDomainBackgroundNodes);
    compareMapAgainstBuiltInGold(tSensitivities, TEST_CONTEXT("FourTriMesh Sensitivities"));
}

TEST_F(OneTriMeshKrinoFixture, OneTriMeshNodeFourFixed)
{
    mDesignDomainBackgroundNodes = std::vector<tpik::BackgroundMeshNodeId>{1U, 2U};
    const auto tSensitivities =
        detail::compute_sensitivities(mKrinoMesh->bulk_data(), mLevelSetFields, mDesignDomainBackgroundNodes);

    tpik::test_utilities::test_sensitivity_map(tSensitivities, kOneTriSensitivityMapNodeFourFixed,
                                               TEST_CONTEXT("OneTriMesh Sensitivities Fixed Node 4"));
}

TEST_F(OneTriMeshKrinoFixture, OneTriMeshNodeOneAndTwoFixed)
{
    mDesignDomainBackgroundNodes = std::vector<tpik::BackgroundMeshNodeId>{4U};
    const auto tSensitivities =
        detail::compute_sensitivities(mKrinoMesh->bulk_data(), mLevelSetFields, mDesignDomainBackgroundNodes);

    tpik::test_utilities::test_sensitivity_map(tSensitivities, kOneTriSensitivityMapNodeOneAndTwoFixed,
                                               TEST_CONTEXT("OneTriMesh Sensitivities Fixed Nodes 1 and 2"));
}

TEST_F(OneTriMeshKrinoFixture, OneTriMeshNodeAllFixed)
{
    mDesignDomainBackgroundNodes = std::vector<tpik::BackgroundMeshNodeId>{};
    const auto tSensitivities =
        detail::compute_sensitivities(mKrinoMesh->bulk_data(), mLevelSetFields, mDesignDomainBackgroundNodes);
    tpik::test_utilities::test_sensitivity_map(tSensitivities, kOneTriSensitivityMapAllNodesFixed,
                                               TEST_CONTEXT("OneTriMesh Sensitivities All Fixed Nodes"));
}

TEST_F(KrinoTestFixture, MakeKrinoWrapperFromAnalysisDomainMeshOneTri)
{
    ASSERT_TRUE(kOneTriMeshFilePath.has_value());
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{
        kOneTriMeshFilePath.value(),
        analysis::AnalysisDomainMesh::BlockScalarField{{1U, {{1U, 0U, .75}, {2U, 1U, -.25}, {4U, 2U, -.25}}}}};
    const auto tFixedBlocks = std::set<std::string>{};
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(
        tAnalysisDomainMesh, tFixedBlocks, tpik::VoidPhase::kExcludeFromMesh, tpik::SnappingParameters{});

    const auto& tSensitivityMap = tKrinoWrapper.sensitivities();
    tpik::test_utilities::test_sensitivity_map(tSensitivityMap, kOneTriSensitivityMap,
                                               TEST_CONTEXT("One tri, one block, no fixed sensitivity map"));
}

TEST_F(KrinoTestFixture, MakeKrinoWrapperFromAnalysisDomainMeshFourTriTwoBlock)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    const auto tAnalysisDomainMesh =
        analysis::AnalysisDomainMesh{kFourTriTwoBlockMeshFilePath.value(),
                                     analysis::AnalysisDomainMesh::BlockScalarField{
                                         {1U, {{1U, 0U, .75}, {2U, 1U, -.25}, {4U, 2U, -.25}, {7U, 3U, .75}}}}};
    const auto tFixedBlocks = std::set<std::string>{"block_2"};
    const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(
        tAnalysisDomainMesh, tFixedBlocks, tpik::VoidPhase::kIncludeInMesh, tpik::SnappingParameters{});

    const auto tFourTriWithFixedBlockSensitivityMap =
        tpik::SensitivityMap{{9, tpik::LevelSetJacobianColumn{{4, 1}, {{0.75, 0.75, 0}, {0.25, 0.25, 0}}, {2, 0}}},
                             {10, tpik::LevelSetJacobianColumn{{7, 2}, {{0.25, 0.25, 0}, {0.75, 0.75, 0}}, {3, 1}}},
                             {8, tpik::LevelSetJacobianColumn{{1, 2}, {{.25, 0, 0}, {0.75, 0, 0}}, {0, 1}}}};

    const auto& tSensitivityMap = tKrinoWrapper.sensitivities();
    tpik::test_utilities::test_sensitivity_map(tSensitivityMap, tFourTriWithFixedBlockSensitivityMap,
                                               TEST_CONTEXT("Four tri, two block, block 2 fixed sensitivity map"));
}

}  // namespace plato::geometry::extension::unittest
