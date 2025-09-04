#include <gtest/gtest.h>

#include <boost/mpi.hpp>

#include "plato/geometry/extension/cubit/test_utilities/CubitTestFixture.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::geometry::extension::cubit::parallel_unittest
{

using test_utilities::CubitTestFixture;

namespace
{
const auto kSensitivityFile = std::filesystem::path{"sensitivity.exo"};

class CubitGeometryConsistency : public test_utilities::CubitTestFixture
{
};

}  // namespace

TEST_F(CubitGeometryConsistency, ParallelGenerateMesh) {}

TEST_F(CubitGeometryConsistency, ParallelJacobian) { checkJacobian(); }

TEST_F(CubitGeometryConsistency, ParallelAdjointJacobian) { checkAdjointJacobian(); }

namespace
{
void test_cubit_output_function(const input_parser::cubit_parameterized_shape& aInput)
{
    const auto tOutputFunction = detail::make_cubit_output(aInput);
    const auto tSolution = linear_algebra::DynamicVector<double>{0.5, 1, 1};
    tOutputFunction(tSolution, library::kOverwriteInfo);

    const auto tRemoveFileList =
        aInput.output_mesh_sensitivities_name.has_value()
            ? std::vector<std::filesystem::path>{kSensitivityFile, aInput.output_file_name.value().mToken}
            : std::vector<std::filesystem::path>{aInput.output_file_name.value().mToken};

    plato::utilities::execute_on_root(boost::mpi::communicator{},
                                      [&tRemoveFileList]() {
                                          plato::test_utilities::test_for_existence_and_remove(
                                              tRemoveFileList, TEST_CONTEXT("Removing aux files."));
                                      });
}

}  // namespace

TEST_F(CubitGeometryConsistency, MakeCubitOutputWithSensitivities)
{
    auto tInput = test_utilities::create_cubit_input_for_test_fixture();
    tInput.output_mesh_sensitivities_name = input_parser::FileName{kSensitivityFile};
    test_cubit_output_function(tInput);
}

TEST_F(CubitGeometryConsistency, MakeCubitOutputWithOutSensitivities)
{
    test_cubit_output_function(test_utilities::create_cubit_input_for_test_fixture());
}
}  // namespace plato::geometry::extension::cubit::parallel_unittest
