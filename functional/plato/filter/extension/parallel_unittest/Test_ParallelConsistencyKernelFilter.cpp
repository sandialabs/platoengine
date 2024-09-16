#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <string>

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesSequentialView.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/RankSplitVector.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::parallel_unittest
{
namespace
{
constexpr std::string_view kMeshFile = "mesh.exo";
constexpr auto kNumRanks = int{4};

boost::mpi::communicator split_comm_world()
{
    auto tWorldComm = boost::mpi::communicator{};
    const auto tRank = tWorldComm.rank();
    const auto tColor = utilities::rank_group_color({1, 3}, utilities::RankNamedType{tRank});
    return tWorldComm.split(tColor.mValue);
}

mesh::Mesh write_generic_mesh_and_load(unsigned int aBaseSize, const boost::mpi::communicator& aWorldCommunicator)
{
    namespace tpi = third_party_integration;
    const auto tCommandGenerator =
        tpi::stk_io::CommandGenerator{{aBaseSize, aBaseSize, aBaseSize}, {-2, -2, -2}, {2, 2, 2}};
    if (aWorldCommunicator.rank() == 0)
    {
        tpi::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    }
    aWorldCommunicator.barrier();
    return mesh::Mesh{kMeshFile};
}

constexpr unsigned int kBaseSizeParallelTest = 4;
class ParallelConsistencyTest : public ::testing::Test
{
   public:
    ParallelConsistencyTest()
        : mWorldComm(boost::mpi::communicator{}),
          mSplitComm(split_comm_world()),
          mMesh(write_generic_mesh_and_load(kBaseSizeParallelTest, mWorldComm))
    {
    }
    ~ParallelConsistencyTest() {}
    void TearDown() override
    {
        mWorldComm.barrier();
        if (mWorldComm.rank() == 0)
        {
            test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
        }
    }
    void check_parallel_consistency_of_sum(const unsigned int aSum, const test_utilities::TestContext& aTestContext)
    {
        auto tOtherRanksSumValue = aSum;
        unsigned int tRankZeroValue = 0;
        if (mWorldComm.rank() == 0)
        {
            tRankZeroValue = aSum;
        }
        else
        {
            boost::mpi::all_reduce(mSplitComm, boost::mpi::inplace(tOtherRanksSumValue), std::plus<unsigned int>());
        }
        boost::mpi::broadcast(mWorldComm, tRankZeroValue, 0);
        EXPECT_EQ(tRankZeroValue, tOtherRanksSumValue) << aTestContext;
    }

   protected:
    boost::mpi::communicator mWorldComm;
    boost::mpi::communicator mSplitComm;
    mesh::Mesh mMesh;
};

std::vector<double> create_linear_space_vector(unsigned int aSize)
{
    if (aSize == 0)
    {
        return {};
    }
    const double tStepSize = 1.0 / aSize;

    std::vector<double> tVector(aSize, tStepSize);
    std::generate_n(tVector.begin() + 1, tVector.size() - 1,
                    [n = tStepSize, dn = tStepSize]() mutable { return n = n + dn; });
    return tVector;
}

auto test_filter_evaluation(const third_party_integration::stk_io::CommandGenerator& aCommandGenerator,
                            const boost::mpi::communicator& aCommunicator)
    -> std::pair<std::vector<mesh::ScalarFieldValue>, std::vector<double> >
{
    const KernelFilter tKernelFilter{mesh::Mesh{kMeshFile}, FilterRadius{1},
                                     input_parser::KernelFilterCenteringTypes::kElementCentered, aCommunicator};

    const std::vector<double> tNodalDensities = create_linear_space_vector(aCommandGenerator.numberOfNodes());
    const std::vector<double> tStdVectorSensitivities =
        create_linear_space_vector(aCommandGenerator.numberOfElements());

    const auto tMesh = mesh::DesignVariablesConversion{mesh::Mesh{kMeshFile}};
    const auto tMeshDesignVariables =
        tMesh.nodalFieldToMeshDesignVariables(mesh::NodalFieldVectorReference{tNodalDensities});
    const auto tResult = tKernelFilter.filter(tMeshDesignVariables);
    const auto tPostFilter = mesh::mesh_design_variables_to_vector(mesh::MeshDesignVariablesSequentialView{tResult});

    const auto tPostSensitivities =
        tKernelFilter
            .jacobianTimesVector(tMeshDesignVariables, linear_algebra::DynamicVector<double>(tStdVectorSensitivities))
            .stdVector();

    return std::pair{tPostFilter, tPostSensitivities};
}

}  // namespace

TEST(ParallelConsistencyKernelFilter, LinearSpace)
{
    const auto tVector = create_linear_space_vector(4);
    ASSERT_EQ(tVector.size(), 4);

    EXPECT_EQ(tVector[0], 0.25);
    EXPECT_EQ(tVector[1], 0.5);
    EXPECT_EQ(tVector[2], 0.75);
    EXPECT_EQ(tVector[3], 1.0);
}

TEST(ParallelConsistencyKernelFilter, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(ParallelConsistencyKernelFilter, CommSplit)
{
    const auto tWorldComm = boost::mpi::communicator{};
    const auto tRank = tWorldComm.rank();

    const auto tSplitComm = split_comm_world();

    if (tRank == 0)
    {
        EXPECT_EQ(tSplitComm.size(), 1);
    }
    else
    {
        EXPECT_EQ(tSplitComm.size(), 3);
    }
}

TEST(ParallelConsistencyKernelFilter, FilterConsistency)
{
    const auto tWorldComm = boost::mpi::communicator{};
    const auto tRank = tWorldComm.rank();
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{{3, 1, 1}, {0, 0, 0}, {3, 1, 1}};

    if (tRank == 0)
    {
        third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    }

    tWorldComm.barrier();
    const auto tSplitComm = split_comm_world();

    const auto [tResultFilter, tResultJV] = test_filter_evaluation(tCommandGenerator, tSplitComm);
    tWorldComm.barrier();

    std::vector<double> tBroadcastResultFilter(tResultFilter.size(), 0);
    std::vector<double> tBroadcastResultJV(tResultJV.size(), 0);

    if (tWorldComm.rank() == 0)
    {
        auto tIDMap = std::vector<std::size_t>{};
        std::tie(tBroadcastResultFilter, tIDMap) = mesh::detail::split_scalar_field_values(tResultFilter);
        tBroadcastResultJV = tResultJV;
    }

    boost::mpi::broadcast(tWorldComm, tBroadcastResultFilter, 0);
    boost::mpi::broadcast(tWorldComm, tBroadcastResultJV, 0);

    for (const auto [tLocalValue, tBroadcastValue] : utilities::Zip{tResultFilter, tBroadcastResultFilter})
    {
        EXPECT_DOUBLE_EQ(tLocalValue.mValue, tBroadcastValue);
    }

    for (const auto [tLocalValue, tBroadcastValue] : utilities::Zip{tResultJV, tBroadcastResultJV})
    {
        EXPECT_DOUBLE_EQ(tLocalValue, tBroadcastValue);
    }

    tWorldComm.barrier();
    if (tRank == 0)
    {
        test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
    }
}

TEST(KernelFilterDetail, CreateLinearMask)
{
    const auto tWorldComm = boost::mpi::communicator{};
    const auto tCommandGenerator =
        third_party_integration::stk_io::CommandGenerator{{4u, 4u, 4u}, {-2, -2, -2}, {2, 2, 2}};
    if (tWorldComm.rank() == 0)
    {
        third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    }
    tWorldComm.barrier();
    const auto tMesh = mesh::Mesh{kMeshFile};
    const FilterRadius tFilterRadius{5};
    {
        const LinearMask tLinearMask = detail::create_linear_mask(
            tMesh, tFilterRadius, input_parser::KernelFilterCenteringTypes::kElementCentered, tWorldComm);
        const auto [tRows, tCols] = tLinearMask.size();
        EXPECT_EQ(tRows, tCommandGenerator.numberOfElements());
        EXPECT_EQ(tCols, tCommandGenerator.numberOfNodes());
    }
    {
        const LinearMask tLinearMask = detail::create_linear_mask(
            tMesh, tFilterRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered, tWorldComm);
        const auto [tRows, tCols] = tLinearMask.size();
        EXPECT_EQ(tRows, tCommandGenerator.numberOfNodes());
        EXPECT_EQ(tCols, tCommandGenerator.numberOfNodes());
    }

    if (tWorldComm.rank() == 0)
    {
        test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
    }
}

TEST_F(ParallelConsistencyTest, ParallelConsistencySearchResults)
{
    constexpr double tSearchRadius = 1;
    const auto tNodalCoordinates = mesh::EntityRetrieval{mMesh}.designDomainNodalCoordinates();
    const auto tRowCenters = mesh::EntityRetrieval{mMesh}.designDomainElementCentroids();

    const auto tSearchResults = detail::distribute_search_vectors_and_stk_search(
        CenterVector{tRowCenters}, NodalVector{tNodalCoordinates}, tSearchRadius, mSplitComm);

    check_parallel_consistency_of_sum(tSearchResults.size(), TEST_CONTEXT("Stk search result size"));
}

TEST_F(ParallelConsistencyTest, ParallelConsistencyCreateTpetraCoordinates)
{
    const auto tNodalCoordinates = mesh::EntityRetrieval{mMesh}.designDomainNodalCoordinates();
    const auto tDistributedNodal = detail::create_tpetravector_coordinates(tNodalCoordinates, mSplitComm);

    check_parallel_consistency_of_sum(
        third_party_integration::tpetra::number_of_local_elements(tDistributedNodal.getMap()),
        TEST_CONTEXT("Tpetra distributed nodal size"));
}

TEST_F(ParallelConsistencyTest, ParallelConsistencySTKSearchPointsFromTpetraVector)
{
    const auto tNodalCoordinates = mesh::EntityRetrieval{mMesh}.designDomainNodalCoordinates();
    const auto tDistributedNodal = detail::create_tpetravector_coordinates(tNodalCoordinates, mSplitComm);

    const auto tLocalSearchPointWithIdentifiers = detail::stk_search_points(tDistributedNodal, mSplitComm.rank());

    check_parallel_consistency_of_sum(tLocalSearchPointWithIdentifiers.size(), TEST_CONTEXT("Stk search points size"));
}

TEST_F(ParallelConsistencyTest, ParallelConsistencySTKSearchSpheresFromTpetraVector)
{
    constexpr double tSearchRadius = 1.5;
    const auto tRowCentersCoordinates = mesh::EntityRetrieval{mMesh}.designDomainElementCentroids();
    const auto tDistributedRowCenters = detail::create_tpetravector_coordinates(tRowCentersCoordinates, mSplitComm);

    check_parallel_consistency_of_sum(
        third_party_integration::tpetra::number_of_local_elements(tDistributedRowCenters.getMap()),
        TEST_CONTEXT("Tpetra distributed row size"));
    const auto tLocalSearchSpheresWithIdentifiers =
        detail::stk_search_spheres(tDistributedRowCenters, SearchRadius{tSearchRadius}, mSplitComm.rank());

    check_parallel_consistency_of_sum(tLocalSearchSpheresWithIdentifiers.size(),
                                      TEST_CONTEXT("Stk search spheres size"));
}

}  // namespace plato::filter::extension::parallel_unittest
