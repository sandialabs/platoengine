#include "plato/geometry/extension/cubit/test_utilities/CubitTestFixture.hpp"

#include <boost/mpi/communicator.hpp>
#include <fstream>
#include <string>
#include <string_view>

#include "plato/geometry/extension/cubit/BoundedApreproVariableParser.hpp"
#include "plato/geometry/extension/cubit/BracedOutput.hpp"
#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension::cubit::test_utilities
{

namespace
{

const std::filesystem::path kJournalFile = "block.jou";
const std::filesystem::path kCubitLogFile = "CubitOutput.log";
constexpr std::string_view kApreproXYScale = "xy_scale";
constexpr std::string_view kApreproHeight = "height";
constexpr std::string_view kApreproXShift = "xshift";

const auto tXYScale =
    input_parser::BoundedApreproVariable{.mName = input_parser::ApreproString{std::string{kApreproXYScale}},
                                         .mInitialValue = 1,
                                         .mBounds = plato::input_parser::Bounds{1, 2}};
const auto tHeight =
    input_parser::BoundedApreproVariable{.mName = input_parser::ApreproString{std::string{kApreproHeight}},
                                         .mInitialValue = 1,
                                         .mBounds = plato::input_parser::Bounds{0.5, 2}};
const auto tXShift =
    input_parser::BoundedApreproVariable{.mName = input_parser::ApreproString{std::string{kApreproXShift}},
                                         .mInitialValue = 0.5,
                                         .mBounds = plato::input_parser::Bounds{0., 2}};

const auto kCubitInput = input_parser::cubit_parameterized_shape{
    /*.mesh_journal_file=*/input_parser::FileName{kJournalFile},
    /*.output_file_name=*/input_parser::FileName{"output.txt"},
    /*.mesh_file_name=*/input_parser::FileName{"mesh.exo"},
    /*.output_mesh_sensitivities_name=*/boost::none,
    /*.aprepro_variable=*/input_parser::BoundedApreproVariableList{{tXYScale, tHeight, tXShift}}};

[[nodiscard]] auto generate_mesh(CubitGeometry& aCubitGeometry) -> std::filesystem::path
{
    plato::utilities::execute_on_root(
        boost::mpi::communicator{}, []() { [[maybe_unused]] const auto tJournalFile = create_meshing_journal_file(); });
    const auto tMesh = aCubitGeometry.generateMesh(linear_algebra::DynamicVector<double>{1.3, 1.5, 1.6});
    return tMesh.mFileName;
}

}  // namespace

CubitTestFixture::CubitTestFixture()
    : mCubitGeometry(CubitGeometry(kCubitInput)), mMeshName(generate_mesh(mCubitGeometry))
{
}

CubitTestFixture::~CubitTestFixture()
{
    plato::utilities::execute_on_root(boost::mpi::communicator{},
                                      [this]()
                                      {
                                          plato::test_utilities::test_for_existence_and_remove(
                                              {kJournalFile, mMeshName, kCubitLogFile},
                                              TEST_CONTEXT("Remove auxilary files."));
                                      });
}

auto create_cubit_input_for_test_fixture() -> plato::input_parser::cubit_parameterized_shape { return kCubitInput; }

auto create_meshing_journal_file() -> std::filesystem::path
{
    std::ofstream tOutfile(std::string{kJournalFile});
    tOutfile << "reset\n";
    tOutfile << "create brick x 1 y 1 z " << BracedOutput{kApreproHeight} << "\n";
    tOutfile << "body all scale x " << BracedOutput{kApreproXYScale} << " y " << BracedOutput{kApreproXYScale} << "\n";
    tOutfile << "move volume 1 x " << BracedOutput{kApreproXShift} << "\n";
    tOutfile << "volume 1 size 1\n";
    tOutfile << "mesh volume 1 \n";
    tOutfile << "block 1 volume 1 \n";
    tOutfile << "block 1 name \"block_1\" \n";
    return kJournalFile;
}

// clang-format off
/*

xySensitivity = [0.5000000000000284 -0.5 0 0.5000000000000284 0.5 0 -0.5 0.5 0 -0.5 -0.5 0 0.5000000000000284 0.5 0 0.5000000000000284 -0.5 0 -0.5 -0.5 0 -0.5 0.5 0 -0.5 -0.5 0 0.5000000000000284 -0.5 0 -0.5 0.5 0 0.5000000000000284 0.5 0];
heightSensitivity = [0 0 0.5 0 0 0.5 0 0 0.5 0 0 0.5 0 0 -0.5 0 0 -0.5 0 0 -0.5 0 0 -0.5 0 0 0 0 0 0 0 0 0 0 0 0];
xshiftSensitivity = [1 0 0 1 0 0 0.9999999999998863 0 0 0.9999999999998863 0 0 1 0 0 1 0 0 0.9999999999998863 0 0 0.9999999999998863 0 0 0.9999999999998863 0 0 1 0 0 0.9999999999998863 0 0 1 0 0];

rowVector = [1:12*3];
JacobianVectorProduct = [dot(rowVector,xySensitivity) dot(rowVector,heightSensitivity) dot(rowVector,xshiftSensitivity)]

JacobianVectorProduct =

         -3.0000  -24.0000  210.0000
*/
// clang-format on
void CubitTestFixture::checkJacobian()
{
    const auto tDesignParameters = linear_algebra::DynamicVector<double>{1.4, 1.5, 1.6};
    const auto tJacobian = mCubitGeometry.jacobianMultiplier(tDesignParameters);

    const auto tEntityCounts = mesh::EntityCounts{mesh::Mesh{mMeshName}};
    const auto tNumberOfNodes = tEntityCounts.numberOfDesignDomainNodes();
    const auto tDimensions = tEntityCounts.spatialDimensions();
    const auto tRowVectorLength = tNumberOfNodes * tDimensions;

    std::vector<double> tRowVec(tRowVectorLength, 1.0);
    std::iota(tRowVec.begin(), tRowVec.end(), 1.0);
    const linear_algebra::DynamicVector<double> tRowVecAsDynamicVector(tRowVec);

    const auto tGold = std::vector<double>{-3, -24, 210};
    const auto tResult = tRowVecAsDynamicVector * tJacobian;

    constexpr double tTolerance = 1e-9;
    plato::test_utilities::expect_container_entries_near(tResult.stdVector(), tGold, tTolerance,
                                                         TEST_CONTEXT("Jacobian"));
}

// clang-format off
/*

columnVector = [1.4 1.5 1.6]';
AdjointJacobian = [xySensitivity' heightSensitivity' xshiftSensitivity'];
AdjointJacobianVectorProduct = AdjointJacobian*columnVector

*/
// clang-format on
void CubitTestFixture::checkAdjointJacobian()
{
    constexpr double tTolerance = 1e-6;
    const auto tDesignParameters = linear_algebra::DynamicVector<double>{1.4, 1.5, 1.6};
    const auto tMeshFile = "mesh.exo";
    const auto tMesh = mesh::DesignVariablesConversion{mesh::Mesh{tMeshFile}};
    const auto tResult = tDesignParameters * mCubitGeometry.adjointJacobianMultiplier(tDesignParameters);
    // clang-format off
    const auto tGold = std::vector<double>{  2.3000, -0.7000,  0.7500, 2.3000, 0.7000, 0.7500, 0.9000, 0.7000, 0.7500, 0.9000, -0.7000,  0.7500, 2.3000, 0.7000, -0.7500,  2.3000, -0.7000,  -0.7500,    0.9000,   -0.7000,   -0.7500,    0.9000,    0.7000,   -0.7500,    0.9000,   -0.7000,  0,  2.3000, -0.7000,  0, 0.9000,  0.7000,  0,  2.3000, 0.7000,  0};
    // clang-format on
    plato::test_utilities::expect_container_entries_near(tResult.stdVector(), tGold, tTolerance,
                                                         TEST_CONTEXT("Adjoint Jacobian"));
}

}  // namespace plato::geometry::extension::cubit::test_utilities
