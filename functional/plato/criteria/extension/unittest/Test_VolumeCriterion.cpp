#include <gtest/gtest.h>

#include <random>
#include <string>

#include "plato/criteria/extension/VolumeCriterion.hpp"
#include "plato/mesh/CoordinateUtilities.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/mesh/test_utilities/MutableCoordinateMesh.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/test_utilities/RandomPerturbationVector.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

constexpr std::string_view kMeshFile = "brick.exo";

class VolumeFractionCriterionHex : public third_party_integration::stk_io::test_utilities::MeshGeneratingTestFixture
{
   protected:
    VolumeFractionCriterionHex()
        : MeshGeneratingTestFixture(kMeshFile, third_party_integration::stk_io::CommandGenerator{})
    {
    }
};

[[nodiscard]] auto mesh_with_constant_controls(const double aControlValue) -> analysis::AnalysisDomainMesh
{
    const auto tMesh = mesh::EntityCounts{mesh::Mesh{kMeshFile}};
    const auto tControls = std::vector<double>(tMesh.numberOfElements(), aControlValue);
    return mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tControls)});
}

void test_volume_criteria_from_ctor_and_function(const VolumeCriterion& aVolumeCriterion,
                                                 const library::CriterionFunction& aFunction,
                                                 const double aGoldVolume)
{
    constexpr double tConstantControls = 0.75;
    const auto tAnalysisDomainMesh = mesh_with_constant_controls(tConstantControls);

    EXPECT_EQ(aVolumeCriterion.f(tAnalysisDomainMesh), aGoldVolume * tConstantControls);
    EXPECT_EQ(aVolumeCriterion.f(tAnalysisDomainMesh),
              aFunction.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh));
}

void test_scaled_and_unscaled_on_ctor_and_function(
    const third_party_integration::stk_io::CommandGenerator& aCommandGenerator)
{
    third_party_integration::stk_io::write_mesh(kMeshFile, aCommandGenerator);
    test_volume_criteria_from_ctor_and_function(
        VolumeCriterion{}, make_volume_constraint_function(/*aIgnoreVoidBlocks=*/true), aCommandGenerator.volume());
    test_volume_criteria_from_ctor_and_function(
        VolumeCriterion{.mScaleFactor = 1.0 / aCommandGenerator.volume(), .mIgnoreVoidBlocks = true},
        make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                 /*aReferenceVolume=*/aCommandGenerator.volume()),
        1);

    EXPECT_TRUE(std::filesystem::remove(kMeshFile));
}

void test_volume_criteria_derivative_from_ctor_and_function(const VolumeCriterion& tVolumeCriterion,
                                                            const library::CriterionFunction& aFunction,
                                                            const std::vector<double>& aGold)
{
    const auto tMesh = mesh::Mesh{kMeshFile};
    const auto tAssignedDensities =
        std::vector<double>{0.5, 0.4, 0.3};  // Not 1 to make certain DF does not depend on them
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tAssignedDensities)});
    const auto tResult = tVolumeCriterion.df(tAnalysisDomainMesh);
    const auto tResultFromFunction = aFunction.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh);

    ASSERT_EQ(tResult.size(), mesh::EntityCounts{tMesh}.numberOfElements());
    ASSERT_EQ(tResult.size(), aGold.size());
    ASSERT_EQ(tResult.size(), tResultFromFunction.size());

    for (unsigned int iIndex = 0; iIndex < tResult.size(); ++iIndex)
    {
        EXPECT_DOUBLE_EQ(tResult[iIndex], aGold[iIndex]);
        EXPECT_DOUBLE_EQ(tResultFromFunction[iIndex], aGold[iIndex]);
    }
}

const std::filesystem::path kPerturbedMeshFilePath{"perturbed_mesh.exo"};
void write_perturbed_mesh_to_file(const linear_algebra::DynamicVector<double>& aPerturbedCoordinates,
                                  const std::filesystem::path& aMeshFilePath)
{
    auto tMesh = mesh::test_utilities::MutableCoordinateMesh{mesh::Mesh{aMeshFilePath}};
    tMesh.updateNodalCoordinates(mesh::dynamic_vector_to_nodal_coordinates(
        aPerturbedCoordinates, mesh::EntityCounts{tMesh}.spatialDimensions()));
    tMesh.writeMeshToDisk(kPerturbedMeshFilePath);
}

struct InputFileIO
{
    InputFileIO(const std::vector<std::string>& aFileLines)
    {
        std::ofstream tTextFile(mFileName);
        tTextFile << "begin volume\n";
        for (const auto& tLine : aFileLines)
        {
            tTextFile << "  ";
            tTextFile << tLine;
            tTextFile << "\n";
        }
        tTextFile << "end\n";
        tTextFile.close();
    }

    ~InputFileIO() { std::filesystem::remove(mFileName); }

    std::filesystem::path mFileName{"dummy_input.i"};
};

}  // namespace

TEST(VolumeCriterion, Volume)
{
    test_scaled_and_unscaled_on_ctor_and_function(third_party_integration::stk_io::CommandGenerator{
        {4, 4, 4}, {0, 0, 0}, {2.0, 2.0, 3.0}, third_party_integration::stk_io::CommandElementType::Hex});
}

TEST(VolumeCriterion, VolumeTetMesh)
{
    test_scaled_and_unscaled_on_ctor_and_function(third_party_integration::stk_io::CommandGenerator{
        {4, 4, 4}, {0, 0, 0}, {1.0, 2.0, 3.0}, third_party_integration::stk_io::CommandElementType::Tet});
}

TEST(VolumeCriterion, VolumeDifferentiatesBetweenNodalAndDensityDesignVariables)
{
    constexpr double tLength{77};
    constexpr double tWidth{86};
    constexpr double tHeight{21};
    constexpr double tVolume{tLength * tWidth * tHeight};
    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
        .mElements = {.mX = 4, .mY = 5, .mZ = 3},
        .mLowerBounds = {.x = 0, .y = 0, .z = 0},
        .mUpperBounds = {.x = tLength, .y = tWidth, .z = tHeight},
        .mType = third_party_integration::stk_io::CommandElementType::Tet};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    // with control field (density topology)
    {
        constexpr double tConstantControlValue{0.5};
        const auto tAnalysisDomainMesh = mesh_with_constant_controls(tConstantControlValue);
        EXPECT_DOUBLE_EQ(VolumeCriterion{}.f(tAnalysisDomainMesh), tConstantControlValue * tVolume);
    }

    // no control field
    {
        const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{.mFileName = kMeshFile, .mBlockScalarField = {}};
        EXPECT_DOUBLE_EQ(VolumeCriterion{}.f(tAnalysisDomainMesh), tVolume);
    }
    std::filesystem::remove(kMeshFile);
}

TEST(VolumeCriterion, DerivativeOfScaledVolumeOnControls)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 3}, {0, 0, 0}, {2.0, 1.0, 3.0}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    {
        const std::vector<double> tGold{2, 2, 2};
        test_volume_criteria_derivative_from_ctor_and_function(
            VolumeCriterion{}, make_volume_constraint_function(/*aIgnoreVoidBlocks=*/true), tGold);
    }

    {
        constexpr double tHexVolume = 2;
        const double tOneOverVolume = 1.0 / tCommandGenerator.volume();
        const std::vector<double> tGold(3, tHexVolume * tOneOverVolume);
        test_volume_criteria_derivative_from_ctor_and_function(
            VolumeCriterion{.mScaleFactor = tOneOverVolume, .mIgnoreVoidBlocks = true},
            make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                     /*aReferenceVolume=*/1. / tOneOverVolume),
            tGold);
    }

    EXPECT_TRUE(std::filesystem::remove(kMeshFile));
}

TEST_F(TwoDThreeBlockMesh, VolumeCriterionWithFixedBlocks_Density)
{
    auto tFixedBlockNames = std::set<std::string>{mBlockNames[0]};
    const auto tMesh = mesh::Mesh{mMeshFilePath, std::move(tFixedBlockNames)};

    const auto tDensityVector = std::vector<double>{0.25, 0.25, 1.0};
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{tDensityVector});

    const auto tResult = VolumeCriterion{}.f(tAnalysisDomainMesh);
    constexpr auto tExpected = double{4.5};
    EXPECT_DOUBLE_EQ(tExpected, tResult);
}

TEST_F(TwoDThreeBlockMesh, GradientVolumeCriterionWithFixedBlocks)
{
    auto tFixedBlockNames = std::set<std::string>{mBlockNames[0], mBlockNames[2]};
    const auto tMesh = mesh::Mesh{mMeshFilePath, std::move(tFixedBlockNames)};
    constexpr auto tNumberOfNodalDensities = 2U;
    const auto tDensityVector = std::vector<double>(tNumberOfNodalDensities, 1.0);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{tDensityVector});

    const auto tResult = VolumeCriterion{}.df(tAnalysisDomainMesh);
    const auto tExpected = mesh::MeshQuantities{tMesh}.designDomainElementVolumes();

    EXPECT_EQ(tExpected, tResult.stdVector());
}

TEST_F(TwoDThreeBlockMesh, VolumeCriterionNodalCoordinateGradientCheck)
{
    auto tMesh = mesh::Mesh{mMeshFilePath};
    const auto tSpatialDimensions = mesh::EntityCounts{tMesh}.spatialDimensions();
    const auto tOriginalCoordinates =
        mesh::nodal_vector_field_to_dynamic_vector(mesh::EntityRetrieval{tMesh}.nodalCoordinates(), tSpatialDimensions);
    const auto tNumNodes = mesh::EntityCounts{tMesh}.numberOfNodes();
    auto tRandomEngine = std::default_random_engine{123};
    const auto tDirection = test_utilities::random_perturbation_vector(tNumNodes * tSpatialDimensions, tRandomEngine);

    const auto tChecker = plato::test_utilities::GradientChecker{
        [mMeshFilePath{mMeshFilePath}](const linear_algebra::DynamicVector<double>& aCoordinates)
        {
            write_perturbed_mesh_to_file(aCoordinates, mMeshFilePath);
            const auto tAnalysisDomainMesh =
                analysis::AnalysisDomainMesh{.mFileName = kPerturbedMeshFilePath, .mBlockScalarField = {}};
            return VolumeCriterion{}.f(tAnalysisDomainMesh);
        },
        [mMeshFilePath{mMeshFilePath}](const linear_algebra::DynamicVector<double>& aCoordinates,
                                       const linear_algebra::DynamicVector<double>& aDirection)
        {
            write_perturbed_mesh_to_file(aCoordinates, mMeshFilePath);
            const auto tAnalysisDomainMesh =
                analysis::AnalysisDomainMesh{.mFileName = kPerturbedMeshFilePath, .mBlockScalarField = {}};
            const auto tGradient = VolumeCriterion{}.df(tAnalysisDomainMesh);
            return tGradient.dot(aDirection);
        }};

    const auto tGradientCheckParameters =
        plato::test_utilities::GradientCheckParameters{.mStepDelta = 0.1, .mNumSteps = 7, .mInitialStepSize = 1.0};
    constexpr auto tFirstOrderTruncationErrorTolerance = 5e-2;

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tOriginalCoordinates, tDirection, tGradientCheckParameters), 0.0,
                tFirstOrderTruncationErrorTolerance)
        << tChecker.table(tOriginalCoordinates, tDirection, tGradientCheckParameters);
    std::filesystem::remove(kPerturbedMeshFilePath);
}

TEST(VolumeCriterion, ParseInputBlock)
{
    // nonexistant file defaults
    {
        const auto tParams = detail::parse_input_block(std::string{"does-not-exist.xml"});
        EXPECT_FALSE(tParams.ignore_void_blocks.has_value());
        EXPECT_FALSE(tParams.reference_volume.has_value());
    }

    // file with just ignore_void_blocks specified
    {
        const auto tInputFileIO = InputFileIO({"ignore_void_blocks false"});
        const auto tParams = detail::parse_input_block(tInputFileIO.mFileName);
        EXPECT_TRUE(tParams.ignore_void_blocks.has_value());
        EXPECT_FALSE(tParams.ignore_void_blocks.value());
        EXPECT_FALSE(tParams.reference_volume.has_value());
    }

    // file with just reference_volume specified
    {
        const auto tInputFileIO = InputFileIO({"reference_volume 1.86"});
        const auto tParams = detail::parse_input_block(tInputFileIO.mFileName);
        EXPECT_FALSE(tParams.ignore_void_blocks.has_value());
        EXPECT_TRUE(tParams.reference_volume.has_value());
        EXPECT_EQ(tParams.reference_volume.value(), 1.86);
    }

    // file with both specified
    {
        const auto tInputFileIO = InputFileIO({"ignore_void_blocks false", "reference_volume 1.86"});
        const auto tParams = detail::parse_input_block(tInputFileIO.mFileName);
        EXPECT_TRUE(tParams.ignore_void_blocks.has_value());
        EXPECT_FALSE(tParams.ignore_void_blocks.value());
        EXPECT_TRUE(tParams.reference_volume.has_value());
        EXPECT_EQ(tParams.reference_volume.value(), 1.86);
    }
}

TEST_F(VolumeFractionCriterionHex, ReferenceVolume)
{
    const auto tDomainVolume = third_party_integration::stk_io::CommandGenerator{}.volume();

    constexpr double tConstantControls = 0.86;
    const auto tAnalysisDomainMesh = mesh_with_constant_controls(tConstantControls);

    EXPECT_EQ(detail::reference_volume(/*aReferenceVolume=*/boost::none, /*aAnalysisDomainMesh=*/tAnalysisDomainMesh),
              tDomainVolume);

    constexpr double tReferenceVolume = 450.0;
    EXPECT_EQ(
        detail::reference_volume(/*aReferenceVolume=*/tReferenceVolume, /*aAnalysisDomainMesh=*/tAnalysisDomainMesh),
        tReferenceVolume);
}

TEST_F(VolumeFractionCriterionHex, ThrowsIfNoReferenceVolumeGivenForNonDensityTopology)
{
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{.mFileName = kMeshFile, .mBlockScalarField = {}};

    const auto tCriterion = make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                                     /*aReferenceVolume=*/boost::none);

    EXPECT_THROW([[maybe_unused]] auto tValue = tCriterion.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
                 , std::runtime_error);

    EXPECT_THROW(
        [[maybe_unused]] auto tValue = tCriterion.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh);
        , std::runtime_error);
}

TEST_F(VolumeFractionCriterionHex, ThrowsIfReferenceVolumeLessThanOrEqualToZero)
{
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{.mFileName = kMeshFile, .mBlockScalarField = {}};

    {
        const auto tCriterion = make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                                         /*aReferenceVolume=*/0.0);
        EXPECT_THROW(
            [[maybe_unused]] auto tValue = tCriterion.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
            , std::runtime_error);
    }
    {
        const auto tCriterion = make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                                         /*aReferenceVolume=*/-1.0);
        EXPECT_THROW(
            [[maybe_unused]] auto tValue = tCriterion.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
            , std::runtime_error);
    }
}

TEST_F(VolumeFractionCriterionHex, ReferenceVolumeScalesAsExpected)
{
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{.mFileName = kMeshFile, .mBlockScalarField = {}};
    {
        constexpr double tReferenceVolume{1.0};
        const auto tCriterion = make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                                         /*aReferenceVolume=*/tReferenceVolume);
        EXPECT_EQ(tCriterion.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh), 1.0);
    }
    {
        constexpr double tReferenceVolume{1.25};
        const auto tCriterion = make_volume_fraction_constraint_function(/*aIgnoreVoidBlocks=*/true,
                                                                         /*aReferenceVolume=*/tReferenceVolume);
        EXPECT_EQ(tCriterion.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh), 0.8);
    }
}

}  // namespace plato::criteria::extension::unittest
