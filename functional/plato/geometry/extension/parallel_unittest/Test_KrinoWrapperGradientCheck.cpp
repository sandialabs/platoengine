#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <stk_util/environment/EnvData.hpp>  //get stk mpi env
#include <vector>

#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
namespace tpik = third_party_integration::krino;
const auto kRectangleMeshFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
const auto kCutPlane = tpik::Plane{{0, -1, 0}, 0.6};
const auto kLevelSetPrimitives = tpik::LevelSetPrimitives{{kCutPlane}, {}};

using tpik::test_utilities::KrinoTestFixture;
using AppendMap = utilities::NamedType<tpik::SensitivityMap, struct AppendMapTag>;
using OtherMap = utilities::NamedType<tpik::SensitivityMap, struct OtherMapTag>;
using AppendLevelSetJacobianColumn =
    utilities::NamedType<tpik::LevelSetJacobianColumn, struct AppendLevelSetJacobianColumnTag>;
using OtherLevelSetJacobianColumn =
    utilities::NamedType<tpik::LevelSetJacobianColumn, struct OtherLevelSetJacobianColumnTag>;

[[nodiscard]] auto merge_level_set_jacobian_columns(AppendLevelSetJacobianColumn aAppendLevelSetJacobianColumn,
                                                    const OtherLevelSetJacobianColumn& aOtherLevelSetJacobianColumn)
    -> tpik::LevelSetJacobianColumn
{
    if (aAppendLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.size() == 2)
    {
        return aAppendLevelSetJacobianColumn.mValue;
    }

    if (aOtherLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.size() == 2)
    {
        return aOtherLevelSetJacobianColumn.mValue;
    }

    auto tLevelSetJacobianColumn = std::move(aAppendLevelSetJacobianColumn).mValue;
    if (tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.size() == 0 ||
        tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.front() !=
            aOtherLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.front())
    {
        tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.push_back(
            aOtherLevelSetJacobianColumn.mValue.mBackgroundMeshNodeIDs.front());
        tLevelSetJacobianColumn.mNodalSensitivities.push_back(
            aOtherLevelSetJacobianColumn.mValue.mNodalSensitivities.front());
        tLevelSetJacobianColumn.mDesignDomainLocalIndex.push_back(
            aOtherLevelSetJacobianColumn.mValue.mDesignDomainLocalIndex.front());
    }

    if (tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.size() > 2)
    {
        std::cout << "larger than I thought..." << std::endl;
    }
    return tLevelSetJacobianColumn;
}

[[nodiscard]] auto merge_sensitivity_maps(AppendMap aAppendMap, const OtherMap& aOtherMap) -> tpik::SensitivityMap
{
    auto tAppendMap = std::move(aAppendMap).mValue;
    for (const auto& [tCutMeshId, tLevelSetJacobianColumn] : aOtherMap.mValue)
    {
        if (const auto tIterator = tAppendMap.find(tCutMeshId); tIterator != tAppendMap.end())
        {
            auto& tAppendLevelSetJacobian = tIterator->second;
            AppendLevelSetJacobianColumn tAppend{tAppendLevelSetJacobian};
            tAppendMap[tCutMeshId] =
                merge_level_set_jacobian_columns(tAppend, OtherLevelSetJacobianColumn{tLevelSetJacobianColumn});
        }
        else
        {
            tAppendMap[tCutMeshId] = tLevelSetJacobianColumn;
        }
    }
    return tAppendMap;
}

[[nodiscard]] auto reduce_sensitivity_maps(const tpik::SensitivityMap& aSensitivityMap) -> tpik::SensitivityMap
{
    const auto tCommunicator = boost::mpi::communicator(
        reinterpret_cast<ompi_communicator_t*>(stk::EnvData::instance().m_parallelComm), boost::mpi::comm_duplicate);
    constexpr int tRootRank = 0;
    std::vector<tpik::SensitivityMap> tGatheredSensitivityMaps;

    boost::mpi::gather(tCommunicator, aSensitivityMap, tGatheredSensitivityMaps, tRootRank);
    tpik::SensitivityMap tRootSensitivityMap;
    if (tCommunicator.rank() == tRootRank)
    {
        for (const auto& tSubSensitivityMap : tGatheredSensitivityMaps)
        {
            tRootSensitivityMap = merge_sensitivity_maps(AppendMap{tRootSensitivityMap}, OtherMap{tSubSensitivityMap});
        }
    }
    boost::mpi::broadcast(tCommunicator, tRootSensitivityMap, tRootRank);
    return tRootSensitivityMap;
}

[[nodiscard]] auto flatten_sensitivities(
    const std::unordered_map<tpik::CutMeshSurfaceNodeId, tpik::LevelSetJacobianColumn>& aMap) -> std::vector<double>
{
    std::vector<double> tFlattenedSensitivity;
    tFlattenedSensitivity.reserve(aMap.size() * 2U * 2U);
    for (const auto& tEntry : aMap)
    {
        for (const auto& tNodalSensitivity : tEntry.second.mNodalSensitivities)
        {
            tFlattenedSensitivity.push_back(tNodalSensitivity.x);
            tFlattenedSensitivity.push_back(tNodalSensitivity.y);
        }
    }

    if (boost::mpi::communicator{}.rank() == 0)
    {
        std::cout << "Flattened sensitivities: ";
        for (const auto& tSens : tFlattenedSensitivity)
        {
            std::cout << tSens << ", ";
        }
        std::cout << std::endl;
    }

    return tFlattenedSensitivity;
}

[[nodiscard]] auto retrieve_cut_node_coordinates(const std::vector<double>& aPerturbedField) -> std::vector<double>
{
    const auto tKrino = test_utilities::make_krino_wrapper_from_vector_values(
        kRectangleMeshFilePath.value(), test_utilities::InitialLevelSetValues{aPerturbedField}, std::nullopt);
    const auto tCutMesh = std::filesystem::path{"cut_mesh.exo"};
    tKrino.writeCutMesh(tCutMesh, tpik::VoidPhase::kIncludeInMesh);

    const auto tOriginalCoordinates = third_party_integration::stk_io::nodal_coordinates(
        *third_party_integration::stk_io::read_mesh_bulk_data(kRectangleMeshFilePath.value()));
    auto tCutCoordinates = third_party_integration::stk_io::nodal_coordinates(
        *third_party_integration::stk_io::read_mesh_bulk_data(tCutMesh));

    const auto tCommunicator = boost::mpi::communicator{};
    tCommunicator.barrier();
    if (tCommunicator.rank() == 0)
    {
        std::filesystem::remove(tCutMesh);
    }
    for (const auto& tCoordinate : tOriginalCoordinates)
    {
        const auto tNewEnd = std::remove(tCutCoordinates.begin(), tCutCoordinates.end(), tCoordinate);
        tCutCoordinates.erase(tNewEnd, tCutCoordinates.end());
    }

    std::vector<double> tFlattenedCoordinates;
    tFlattenedCoordinates.reserve(tCutCoordinates.size() * 2U);
    for (const auto& tCutCoordinate : tCutCoordinates)
    {
        tFlattenedCoordinates.push_back(tCutCoordinate.x);
        tFlattenedCoordinates.push_back(tCutCoordinate.y);
    }

    if (tCommunicator.rank() == 0)
    {
        std::cout << "Flattened coordinates: ";
        for (const auto& tCoor : tFlattenedCoordinates)
        {
            std::cout << tCoor << ", ";
        }
        std::cout << std::endl;
    }

    return tFlattenedCoordinates;
}

[[nodiscard]] auto retrieve_sensitivities(const std::vector<double>& aPerturbedField) -> std::vector<double>
{
    const auto tCommunicator = boost::mpi::communicator{};
    std::vector<double> tFlattenedSensitivity;
    std::cout << "here" << std::endl;
    const auto tWrapper = test_utilities::make_krino_wrapper_from_vector_values(
        kRectangleMeshFilePath.value(), test_utilities::InitialLevelSetValues{aPerturbedField}, std::nullopt);
    std::cout << "retrieving sensitivities..." << std::endl;
    tFlattenedSensitivity = flatten_sensitivities(reduce_sensitivity_maps(tWrapper.sensitivities()));

    // std::cout << "broadcasting " << std::endl;
    // boost::mpi::broadcast(tCommunicator, tFlattenedSensitivity, 0);
    return tFlattenedSensitivity;
}

}  // namespace

TEST(KrinoWrapperGradientCheck, MeshFilePath) { ASSERT_TRUE(kRectangleMeshFilePath.has_value()); }

/* This is a specialized example - the 2d rectangle mesh has a linear level set profile. The 0 contour happens in a nice
 * place where mesh changes are well defined. The direction vector is uniform, so the level set fields all move up and
 * the boundary moves linearly in y-hat direction. The numerical gradient is exact for even large step size and so the
 * GC really produces the truncation error region. */
TEST_F(KrinoTestFixture, CheckGradientForPerturbationOfLevelSetPlane)
{
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX) -> double
    {
        const auto tCutCoordinates = retrieve_cut_node_coordinates(aX.stdVector());
        std::cout << "Accumulated coordinates: " << std::accumulate(tCutCoordinates.begin(), tCutCoordinates.end(), 0.0)
                  << std::endl;
        return std::accumulate(tCutCoordinates.begin(), tCutCoordinates.end(), 0.0);
    };
    const auto tDf = [](const linear_algebra::DynamicVector<double>& aX,
                        const linear_algebra::DynamicVector<double>& aV) -> double
    {
        const auto tSensitivities = linear_algebra::DynamicVector<double>{retrieve_sensitivities(aX.stdVector())};
        std::cout << "Sensitivities dot av: " << tSensitivities.dot(aV) << std::endl;
        return tSensitivities.dot(aV);
    };

    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{0.5, 7, 1.0};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{tF, tDf};

    const auto tX = linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
        kRectangleMeshFilePath.value(), kLevelSetPrimitives, std::nullopt)};

    const auto tDirection = linear_algebra::DynamicVector(std::vector<double>(tX.size(), 0.1));

    const auto tErrors = tGradientCheck.finiteDifferenceErrors(tX, tDirection, tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());

    constexpr auto tTolerance = 1e-12;
    for (const auto& tTruncationError : tErrors)
    {
        ASSERT_NEAR(tTruncationError.mValue, 0.0, tTolerance)
            << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
    }

    auto tMonotonicIterator =
        std::adjacent_find(tErrors.begin(), tErrors.end(),
                           [](const auto aSecond, const auto aFirst) { return aSecond.mValue >= aFirst.mValue; });
    EXPECT_EQ(tMonotonicIterator, tErrors.end()) << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
}

}  // namespace plato::geometry::extension::unittest
