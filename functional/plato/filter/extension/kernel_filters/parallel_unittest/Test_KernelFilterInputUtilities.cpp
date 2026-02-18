#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/KernelFilterInputUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReflectFilter.hpp"
#include "plato/filter/extension/kernel_filters/RevolveFilter.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/ReflectionMeshTestUtility.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::filter::extension::kernel_filters::parallel_unittest
{

TEST(ParallelKernelFilterInputUtilities, ValidateAllTargetDomainFindSourceDomain)
{
    constexpr auto tNumRanks = int{4};
    const auto tCommunicator = boost::mpi::communicator{};
    ASSERT_EQ(tNumRanks, tCommunicator.size());
    const auto tSourceMeshFile = std::filesystem::path{"source.exo"};
    const auto tReflectInput = test_utilities::make_filter_input<input_parser::positive_octant_reflect_filter>(
        1.5, input_parser::KernelFilterCenteringTypes::kNodeCentered);

    utilities::execute_on_root(tCommunicator,
                               [&tSourceMeshFile, &tReflectInput]()
                               {
                                   test_utilities::create_mesh_for_reflection_filter(tSourceMeshFile, {0, 0, 0});
                                   test_utilities::create_mesh_for_reflection_filter(
                                       tReflectInput.target_mesh_name.value().mToken, {-1, -1, -1});
                               });

    const auto tErrorMessage = validate_all_target_domain_find_source_domain(
        tReflectInput, tSourceMeshFile,
        [](const input_parser::positive_octant_reflect_filter& aInput,
           const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> ReflectFilterType
        { return make_positive_octant_reflect_filter_type(aInput, aAnalysisDomainMesh); },
        tCommunicator);
    EXPECT_FALSE(tErrorMessage.has_value());

    utilities::execute_on_root(tCommunicator,
                               [&tSourceMeshFile, &tReflectInput]()
                               {
                                   plato::test_utilities::test_for_existence_and_remove(
                                       {tSourceMeshFile, tReflectInput.target_mesh_name.value().mToken},
                                       TEST_CONTEXT("Files written to disk properly."));
                               });
}

}  // namespace plato::filter::extension::kernel_filters::parallel_unittest
