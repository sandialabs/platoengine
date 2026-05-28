#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/process_manager/extension/test_utilities/NodalResultFilterUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::parallel_unittest
{
namespace
{

using NodalResultParallelFixture = test_utilities::NodalResultFilterRunFixture;
}  // namespace

TEST_F(NodalResultParallelFixture, CreateAndRunUseAllRanksInFilter)
{
    const auto tNumberOfProcessorsForFilter = boost::mpi::communicator{}.size();
    run_and_check_nodal_filter_result(std::nullopt, TEST_CONTEXT("All ranks in filter"), tNumberOfProcessorsForFilter);
}

TEST_F(NodalResultParallelFixture, CreateAndRunUseFewerRanksInFilter)
{
    const auto tNumberOfProcessorsForFilter = boost::mpi::communicator{}.size() / 2;
    run_and_check_nodal_filter_result(std::nullopt, TEST_CONTEXT("Fewer ranks in filter than total"),
                                      tNumberOfProcessorsForFilter);
}

}  // namespace plato::process_manager::extension::parallel_unittest
