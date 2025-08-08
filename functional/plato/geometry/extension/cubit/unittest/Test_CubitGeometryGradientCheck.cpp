#include <gtest/gtest.h>

#include <fstream>
#include <numeric>
#include <string_view>

#include "plato/geometry/extension/cubit/BracedOutput.hpp"
#include "plato/geometry/extension/cubit/CubitGeometry.hpp"
#include "plato/geometry/extension/cubit/CubitGeometryCommonUtilities.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::geometry::extension::cubit::unittest
{

namespace
{
constexpr std::string_view kJournalFile = "quarter_ring.jou";
constexpr std::string_view kOutputMeshName = "mesh.exo";
constexpr std::string_view kOuterRadius = "outer_radius";
constexpr std::string_view kThickness = "thickness";

[[nodiscard]] auto mesh_curve_command(const std::string& aSelectorString, const unsigned int aInterval) -> std::string
{
    std::stringstream tMeshCurveCommands;
    tMeshCurveCommands << aSelectorString << "  interval " << aInterval << std::endl;
    tMeshCurveCommands << aSelectorString << "  scheme equal" << std::endl;
    tMeshCurveCommands << "mesh " << aSelectorString << std::endl;
    return tMeshCurveCommands.str();
}

void create_journal_file()
{
    std::ofstream tOutFile(std::string{kJournalFile});
    const auto tInnerRadius = BracedOutput{utilities::concatenate(kOuterRadius, "-", kThickness)};

    tOutFile << "create Cylinder height 1 radius " << BracedOutput{kOuterRadius} << std::endl;
    tOutFile << "volume 1 name \"cyl\"" << std::endl;
    tOutFile << "webcut volume with name \"cyl\" with cylinder radius " << tInnerRadius << " axis z" << std::endl;
    tOutFile << "delete volume with name \"cyl@A\"" << std::endl;
    tOutFile << "volume all size " << BracedOutput{kThickness} << std::endl;
    tOutFile << "section volume all xplane" << std::endl;
    tOutFile << "section volume all yplane" << std::endl;

    std::stringstream tCurveSelector;
    tCurveSelector << "curve with z_coord == 0 and y_coord == 0 and x_coord == " << tInnerRadius;
    tOutFile << mesh_curve_command(tCurveSelector.str(), 4U);
    tCurveSelector.clear();
    tCurveSelector << " curve with z_coord > 0 and x_coord <= " << tInnerRadius << " and y_coord <= " << tInnerRadius;

    tOutFile << mesh_curve_command(tCurveSelector.str(), 10U);

    tOutFile << "mesh volume all" << std::endl;
    tOutFile << "block 1 add volume all" << std::endl;
}

const auto tOuterRadius = input_parser::BoundedApreproVariable{input_parser::ApreproString{std::string{kOuterRadius}},
                                                               1.5, plato::input_parser::Bounds{1, 2}};
const auto tThickness = input_parser::BoundedApreproVariable{input_parser::ApreproString{std::string{kThickness}}, 0.2,
                                                             plato::input_parser::Bounds{0.1, 0.5}};

const auto kInput = input_parser::cubit_parameterized_shape{
    /*.mesh_journal_file=*/input_parser::FileName{std::string{kJournalFile}},
    /*.output_file_name=*/input_parser::FileName{"output.txt"},
    /*.mesh_file_name=*/input_parser::FileName{"mesh.exo"},
    /*.output_mesh_sensitivities_name=*/boost::none,
    /*.aprepro_variables=*/input_parser::BoundedApreproVariableList{{tOuterRadius, tThickness}}};

}  // namespace

TEST(CubitGeometry, CheckGradient)
{
    create_journal_file();
    auto tCubitGeometry = CubitGeometry(kInput);

    const auto tX = cubit::initial_guess(kInput);
    const auto tDirection = linear_algebra::DynamicVector(std::vector<double>(tX.size(), 0.10));
    const auto tF = [&tCubitGeometry](const linear_algebra::DynamicVector<double>& aX) -> double
    {
        const auto tAnalysisDomainMesh = tCubitGeometry.generateMesh(aX);
        const auto tCoordinates = mesh::EntityRetrieval{mesh::Mesh{tAnalysisDomainMesh.mFileName}}.nodalCoordinates();
        const auto tSumCoordinate = std::accumulate(tCoordinates.begin(), tCoordinates.end(),
                                                    third_party_integration::common::Coordinate{0, 0, 0});
        return tSumCoordinate.x + tSumCoordinate.y + tSumCoordinate.z;
    };

    const auto tDf = [&tCubitGeometry](const linear_algebra::DynamicVector<double>& aX,
                                       const linear_algebra::DynamicVector<double>& aV) -> double
    {
        const auto tJacobianMultiplier = tCubitGeometry.jacobianMultiplier(aX);
        const auto tAdjointJacobianMultiplier = tCubitGeometry.adjointJacobianMultiplier(aX);
        const auto tAnalysisDomainMesh = tCubitGeometry.generateMesh(aX);
        const auto tCoordinates = mesh::EntityRetrieval{mesh::Mesh{tAnalysisDomainMesh.mFileName}}.nodalCoordinates();
        const auto tOnesVector =
            linear_algebra::DynamicVector<double>{std::vector<double>(tCoordinates.size() * 3, 1.0)};

        const auto tJacobianVectorProduct =
            linear_algebra::DynamicVector<double>{tJacobianMultiplier.mVectorTimesJacobianFunction(tOnesVector)}.dot(
                aV);

        const auto tAdjointJacobianVectorProduct =
            linear_algebra::DynamicVector<double>{tAdjointJacobianMultiplier.mValue.mVectorTimesJacobianFunction(aV)}
                .dot(tOnesVector);

        EXPECT_NEAR(tJacobianVectorProduct, tAdjointJacobianVectorProduct, 1e-11);

        return tJacobianVectorProduct;
    };

    const auto tGradientCheckParameters =
        plato::test_utilities::GradientCheckParameters{.mStepDelta = 0.5, .mNumSteps = 6, .mInitialStepSize = 1};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{tF, tDf};

    const auto tErrors = tGradientCheck.finiteDifferenceErrors(tX, tDirection, tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());

    constexpr auto tTolerance = 5e-12;
    for (const auto& tTruncationError : tErrors)
    {
        ASSERT_NEAR(tTruncationError.mValue, 0.0, tTolerance)
            << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
    }

    test_utilities::test_for_existence_and_remove({kJournalFile, kOutputMeshName},
                                                  TEST_CONTEXT("journal and mesh file existence"));
}

}  // namespace plato::geometry::extension::cubit::unittest
