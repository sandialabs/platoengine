#include <gtest/gtest.h>

#include <string_view>
#include <unordered_map>

#include "plato/geometry/extension/cubit/CubitGeometry.hpp"
#include "plato/geometry/extension/cubit/CubitGeometryCommonUtilities.hpp"
#include "plato/geometry/extension/cubit/test_utilities/CubitTestFixture.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::geometry::extension::cubit::unittest
{

using test_utilities::CubitTestFixture;

namespace
{

constexpr double kTolerance = 1e-9;
const auto kCubitLogFile = std::filesystem::path{"CubitOutput.log"};
}  // namespace

TEST(CubitDetail, ValidateMeshJournalFile)
{
    auto tInput = test_utilities::create_cubit_input_for_test_fixture();
    EXPECT_TRUE(detail::validate_mesh_journal_file(tInput).has_value())
        << "Invalid: File specified but missing from disk.";
    const auto tJournalFile = test_utilities::create_meshing_journal_file();
    EXPECT_FALSE(detail::validate_mesh_journal_file(tInput).has_value()) << "Valid: File specified and on disk.";
    tInput.mesh_journal_file = boost::none;
    EXPECT_TRUE(detail::validate_mesh_journal_file(tInput).has_value()) << "Invalid: missing required parameter.";
    plato::test_utilities::test_for_existence_and_remove({tJournalFile}, TEST_CONTEXT("Removing journal file"));
}

TEST(CubitDetail, WriteExodusMesh)
{
    const auto tJournalFile = test_utilities::create_meshing_journal_file();
    auto tCubit = CubitWrapper{};
    const auto tCubitInput = test_utilities::create_cubit_input_for_test_fixture();
    const auto tExodusMeshName = std::filesystem::path{tCubitInput.mesh_file_name.value().mToken};

    const auto tVariables = initialize_variables(tCubitInput);

    detail::write_exodus_mesh(tCubit, tVariables, detail::JournalFile{tJournalFile},
                              detail::ExodusFile{tExodusMeshName});
    plato::test_utilities::test_for_existence_and_remove({tExodusMeshName, tJournalFile},
                                                         TEST_CONTEXT("Remove journal file and exodus mesh."));
    const auto tStepFile = std::filesystem::path{"StepFile.stp"};
    tCubit.exportStepFile(tStepFile);
    plato::test_utilities::test_for_existence_and_remove({tStepFile, "step_export.log", kCubitLogFile},
                                                         TEST_CONTEXT("Remove step file."));
}

TEST(CubitDetail, DetermineSensitivities)
{
    const auto tJournalFile = test_utilities::create_meshing_journal_file();
    auto tCubit = CubitWrapper{};
    const auto tCubiInput = test_utilities::create_cubit_input_for_test_fixture();
    const auto tVariables = initialize_variables(tCubiInput);
    restore_aprepro_variables(tCubit, tVariables);
    const auto tSensitivities = detail::sensitivities(tCubit, tVariables, tJournalFile);

    ASSERT_EQ(tSensitivities.size(), tVariables.size());

    {
        const auto tSensitivity = tSensitivities[0];
        for (const auto tValue : tSensitivity)
        {
            EXPECT_NEAR(std::abs(tValue.second.x), 0.5, kTolerance);
            EXPECT_NEAR(std::abs(tValue.second.y), 0.5, kTolerance);
            EXPECT_NEAR(tValue.second.z, 0.0, kTolerance);
        }
    }
    {
        const auto tSensitivity = tSensitivities[1];
        for (const auto tValue : tSensitivity)
        {
            const auto tAbsoluteZ = std::abs(tValue.second.z);
            EXPECT_NEAR(tAbsoluteZ, 0.5, kTolerance) << "Height expansion";
            EXPECT_NEAR(tValue.second.x, 0., kTolerance);
            EXPECT_NEAR(tValue.second.y, 0., kTolerance);
        }
    }
    {
        const auto tSensitivity = tSensitivities[2];
        for (const auto tValue : tSensitivity)
        {
            EXPECT_NEAR(tValue.second.x, 1., kTolerance) << "X Shift";
            EXPECT_NEAR(tValue.second.y, 0., kTolerance);
            EXPECT_NEAR(tValue.second.z, 0., kTolerance);
        }
    }
    plato::test_utilities::test_for_existence_and_remove({tJournalFile, kCubitLogFile},
                                                         TEST_CONTEXT("Removing journal file"));
}

TEST_F(CubitTestFixture, SerialGenerateMesh) {}

TEST_F(CubitTestFixture, SerialOutputMeshSensitivities)
{
    auto tInput = test_utilities::create_cubit_input_for_test_fixture();
    const auto tSensitivityFile = std::filesystem::path{"sensitivity.exo"};
    tInput.output_mesh_sensitivities_name = input_parser::FileName{tSensitivityFile};
    auto tCubitGeometry = CubitGeometry{tInput};
    const auto tSolution = linear_algebra::DynamicVector<double>({1.5, 0.75, 1.0});
    const auto tAnalysisDomainMesh = tCubitGeometry.generateMesh(tSolution);

    const auto tOutputInfo = output::OutputInfo{true, 0};
    const auto tMeshOutput = mesh::mesh_output(mesh::output_mode(tOutputInfo.mOverwrite), tAnalysisDomainMesh,
                                               tSensitivityFile, tOutputInfo.mIteration);

    tCubitGeometry.outputMeshSensitivities(*tMeshOutput);
    plato::test_utilities::test_for_existence_and_remove({tSensitivityFile}, TEST_CONTEXT("Removing sensitivity file"));
}

TEST_F(CubitTestFixture, SerialJacobian) { checkJacobian(); }

TEST_F(CubitTestFixture, SerialAdjointJacobian) { checkAdjointJacobian(); }

}  // namespace plato::geometry::extension::cubit::unittest
