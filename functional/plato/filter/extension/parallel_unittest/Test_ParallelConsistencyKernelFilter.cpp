#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <string>

#include "plato/core/MeshProxy.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/RankSplitVector.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
constexpr std::string_view kMeshFile = "mesh.exo";
constexpr auto kNumRanks = int{4};

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

std::pair<std::vector<double>, std::vector<double> > test_filter_evaluation(
    const third_party_integration::stk_io::CommandGenerator& aCommandGenerator,
    const boost::mpi::communicator& aCommunicator)
{
    const KernelFilter tKernelFilter{kMeshFile, FilterRadius{1},
                                     input_parser::KernelFilterCenteringTypes::kElementCentered, aCommunicator};

    const std::vector<double> tNodalDensities = create_linear_space_vector(aCommandGenerator.numberOfNodes());
    const std::vector<double> tStdVectorSensitivities =
        create_linear_space_vector(aCommandGenerator.numberOfElements());

    const core::MeshProxy tMeshProxy{kMeshFile, tNodalDensities};
    const auto tPostFilter = tKernelFilter.filter(tMeshProxy).mNodalDensities;
    const auto tPostSensitivities =
        tKernelFilter.jacobianTimesVector(tMeshProxy, linear_algebra::DynamicVector<double>(tStdVectorSensitivities))
            .stdVector();

    return {tPostFilter, tPostSensitivities};
}

boost::mpi::communicator split_coms()
{
    auto tWorldComm = boost::mpi::communicator{};
    const auto tRank = tWorldComm.rank();
    const auto tColor = utilities::rank_group_color({1, 3}, utilities::RankNamedType{tRank});
    return tWorldComm.split(tColor.mValue);
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

    const auto tSplitComm = split_coms();

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
        third_party_integration::stk_io::write_mesh(kMeshFile,
                                                    third_party_integration::stk_io::generate_mesh(tCommandGenerator));
    }

    tWorldComm.barrier();
    const auto tSplitComm = split_coms();

    const auto [tResultFilter, tResultJV] = test_filter_evaluation(tCommandGenerator, tSplitComm);
    tWorldComm.barrier();

    std::vector<double> tBroadcastResultFilter(tResultFilter.size(), 0);
    std::vector<double> tBroadcastResultJV(tResultJV.size(), 0);

    if (tWorldComm.rank() == 0)
    {
        tBroadcastResultFilter = tResultFilter;
        tBroadcastResultJV = tResultJV;
    }

    boost::mpi::broadcast(tWorldComm, tBroadcastResultFilter, 0);
    boost::mpi::broadcast(tWorldComm, tBroadcastResultJV, 0);

    for (const auto [tLocalValue, tBroadcastValue] : utilities::Zip{tResultFilter, tBroadcastResultFilter})
    {
        EXPECT_DOUBLE_EQ(tLocalValue, tBroadcastValue);
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
        third_party_integration::stk_io::write_mesh(kMeshFile,
                                                    third_party_integration::stk_io::generate_mesh(tCommandGenerator));
    }
    tWorldComm.barrier();
    const FilterRadius tFilterRadius{5};
    {
        const LinearMask tLinearMask = detail::create_linear_mask(
            kMeshFile, tFilterRadius, input_parser::KernelFilterCenteringTypes::kElementCentered, tWorldComm);
        const auto [tRows, tCols] = tLinearMask.size();
        EXPECT_EQ(tRows, tCommandGenerator.numberOfElements());
        EXPECT_EQ(tCols, tCommandGenerator.numberOfNodes());
    }
    {
        const LinearMask tLinearMask = detail::create_linear_mask(
            kMeshFile, tFilterRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered, tWorldComm);
        const auto [tRows, tCols] = tLinearMask.size();
        EXPECT_EQ(tRows, tCommandGenerator.numberOfNodes());
        EXPECT_EQ(tCols, tCommandGenerator.numberOfNodes());
    }

    if (tWorldComm.rank() == 0)
    {
        test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
    }
}

}  // namespace plato::filter::extension::unittest
