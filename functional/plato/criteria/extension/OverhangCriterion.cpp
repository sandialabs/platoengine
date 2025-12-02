#include "plato/criteria/extension/OverhangCriterion.hpp"

#include <boost/spirit/include/qi.hpp>
#include <filesystem>
#include <numbers>
#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshSidesets.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"
#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"
#include "plato/third_party_integration/krino/TriangleUtilities.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::input_parser
{
template <>
struct InputTypeName<overhang_criterion>
{
    static constexpr inline const char* name = "overhang";
};
}  // namespace plato::input_parser

namespace plato::criteria::extension
{
namespace
{
constexpr auto kCriterionValuePrecision = 8U;
constexpr auto kCriterionValueFieldWidth = kCriterionValuePrecision + 1U;

using Registration =
    library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;

const auto kConfiguration = services::CriterionConfiguration{
    .mName = std::string{OverhangCriterion::kCriterionName}, .mIsParallelized = false, .mIsScalar = true};

[[maybe_unused]] static auto kOverhangRegistration =
    Registration{library::builtin_criterion_registration_name(OverhangCriterion::kCriterionName),
                 [](const library::CriterionInput& aCriterionInput)
                 {
                     return library::FunctionWithConfiguration{.mFunction = make_overhang_function(aCriterionInput),
                                                               .mConfiguration = kConfiguration};
                 }};
}  // namespace

double OverhangCriterion::f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    using namespace plato::third_party_integration::krino;

    auto tLogger = services::component_logger(mComponentType, mName);
    tLogger.logInfo("Evaluating criterion");

    const std::vector<SensitivityTriangle> tTriangles =
        detail::get_triangles_to_evaluate_over(aAnalysisDomainMesh.mFileName, mEvaluationSidesets);

    const double tReturnValue =
        std::accumulate(tTriangles.begin(), tTriangles.end(), 0.0, [&](double aCurrentSum, SensitivityTriangle aCurTri)
                        { return aCurrentSum + detail::area_weighted_overhang_from_triangle(aCurTri, *this); });
    tLogger.logInfo(
        "Evaluation complete. Criterion value = " +
        utilities::to_string(
            utilities::FixedWidthFloatingPointOutput<double, kCriterionValuePrecision, kCriterionValueFieldWidth>{
                tReturnValue}));
    return tReturnValue;
}

linear_algebra::DynamicVector<double> OverhangCriterion::df(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    using namespace plato::third_party_integration::krino;

    auto tLogger = services::component_logger(mComponentType, mName);
    tLogger.logInfo("Evaluating criterion gradient");

    const std::vector<SensitivityTriangle> tTriangles =
        detail::get_triangles_to_evaluate_over(aAnalysisDomainMesh.mFileName, mEvaluationSidesets);

    const std::unordered_map<GlobalNodeID, Sensitivity> tGradientMap =
        detail::calculate_gradient_map_from_triangles(tTriangles, *this);

    const auto tEntityRetrievalMesh = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const std::vector<size_t> tAllNodeIds = tEntityRetrievalMesh.allNodeIDs();

    auto tGradientVector = detail::get_full_gradient_vector_from_gradient_map(tGradientMap, tAllNodeIds);

    const double tGradientNorm =
        std::sqrt(std::inner_product(tGradientVector.begin(), tGradientVector.end(), tGradientVector.begin(), 0.0));

    tLogger.logInfo(
        "Gradient evaluation complete. Criterion gradient norm = " +
        utilities::to_string(
            utilities::FixedWidthFloatingPointOutput<double, kCriterionValuePrecision, kCriterionValueFieldWidth>{
                tGradientNorm}));

    return linear_algebra::DynamicVector<double>(std::move(tGradientVector));
}

namespace
{
constexpr double kTransitionWidth{0.1};
constexpr double kOverhangAngleInDegrees{45.0};
const third_party_integration::common::Vector3 kBuildDirection{0.0, 0.0, 1.0};

[[nodiscard]] std::vector<std::string> to_sideset_list(
    const boost::optional<input_parser::FileList>& aAdditionalSidesets)
{
    std::vector<std::string> tSidesetVector{std::string{third_party_integration::krino::get_interface_sideset_name()}};
    if (aAdditionalSidesets.has_value())
    {
        tSidesetVector.insert(tSidesetVector.end(), aAdditionalSidesets.value().begin(),
                              aAdditionalSidesets.value().end());
    }
    return tSidesetVector;
}

[[nodiscard]] third_party_integration::common::Vector3 to_common_vector_or_default(
    const boost::optional<input_parser::Point>& aDirection)
{
    if (aDirection.has_value())
    {
        return third_party_integration::common::Vector3{aDirection.value().mX, aDirection.value().mY,
                                                        aDirection.value().mZ};
    }
    return kBuildDirection;
}
}  // namespace

OverhangCriterion::OverhangCriterion(const input_parser::overhang_criterion& aInputParams,
                                     const library::CriterionInput& aCriterionInput)
    : mTransitionWidth(aInputParams.transition_width.value_or(kTransitionWidth)),
      mBuildDirection(to_common_vector_or_default(aInputParams.build_direction)),
      mOverhangAngleThreshold(detail::convert_angle_to_threshold_value(
          aInputParams.overhang_angle_in_degrees.value_or(kOverhangAngleInDegrees))),
      mEvaluationSidesets(to_sideset_list(aInputParams.additional_evaluation_sidesets)),
      mComponentType{aCriterionInput.mComponentType},
      mName{aCriterionInput.mName}
{
}

OverhangCriterion::OverhangCriterion(const double aTransitionWidth,
                                     const third_party_integration::common::Vector3& aBuildDirection,
                                     const double aOverhangAngleThreshold)
    : mTransitionWidth(aTransitionWidth),
      mBuildDirection(aBuildDirection),
      mOverhangAngleThreshold(aOverhangAngleThreshold)
{
}

auto make_overhang_function(const library::CriterionInput& aCriterionInput) -> library::CriterionFunction
{
    if (aCriterionInput.mInputFiles.list().mList.size() == 0)
    {
        throw utilities::Exception{"You must specify an overhang criterion input file."};
    }
    const auto& tInputParams = detail::parse_input_deck(aCriterionInput.mInputFiles.list().mList[0]);
    return core::make_function_with_first_derivative(
        [tInputParams, aCriterionInput](const analysis::AnalysisDomainMesh& mesh)
        { return OverhangCriterion(tInputParams, aCriterionInput).f(mesh); },
        [tInputParams, aCriterionInput](const analysis::AnalysisDomainMesh& mesh)
        { return OverhangCriterion(tInputParams, aCriterionInput).df(mesh); });
}

namespace detail
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::krino;

[[nodiscard]] double convert_angle_to_threshold_value(const double aAngle)
{
    return -std::cos(aAngle * std::numbers::pi / 180.0);
}

[[nodiscard]] auto get_triangles_to_evaluate_over(const std::string& aMeshFileName,
                                                  const std::vector<std::string>& aEvaluationSidesetNames)
    -> std::vector<SensitivityTriangle>
{
    const auto tMesh = mesh::MeshSidesets{mesh::Mesh{aMeshFileName}};
    std::vector<SensitivityTriangle> tTriangles;
    for (const auto& tCurEvaluationSideset : aEvaluationSidesetNames)
    {
        const std::vector<SensitivityTriangle> tCurTriangles = tMesh.sidesetTriangles(tCurEvaluationSideset);
        tTriangles.insert(tTriangles.end(), tCurTriangles.begin(), tCurTriangles.end());
    }
    return tTriangles;
}

[[nodiscard]] auto calculate_gradient_map_from_triangles(const std::vector<SensitivityTriangle>& aTriangles,
                                                         const OverhangCriterion& aOverhangCriterion)
    -> std::unordered_map<GlobalNodeID, Sensitivity>
{
    std::unordered_map<GlobalNodeID, Sensitivity> tGradientMap;
    //  Initialize all future map entries to 0.0
    std::for_each(aTriangles.begin(), aTriangles.end(),
                  [&tGradientMap](const auto& aCurTriangle)
                  {
                      tGradientMap[aCurTriangle.mNodes[0].first] = {.x = 0.0, .y = 0.0, .z = 0.0};
                      tGradientMap[aCurTriangle.mNodes[1].first] = {.x = 0.0, .y = 0.0, .z = 0.0};
                      tGradientMap[aCurTriangle.mNodes[2].first] = {.x = 0.0, .y = 0.0, .z = 0.0};
                  });
    //  Accumulate gradient contributions from all triangles
    std::for_each(aTriangles.begin(), aTriangles.end(),
                  [&tGradientMap, aOverhangCriterion](const auto& aCurTriangle)
                  {
                      const TriangleGradient tCurTriGradient =
                          detail::get_gradient_contribution_for_triangle(aCurTriangle, aOverhangCriterion);
                      std::for_each(tCurTriGradient.begin(), tCurTriGradient.end(),
                                    [&tGradientMap](const auto& aCurNodeGradient)
                                    { tGradientMap[aCurNodeGradient.first] += aCurNodeGradient.second; });
                  });
    return tGradientMap;
}

[[nodiscard]] auto get_full_gradient_vector_from_gradient_map(
    const std::unordered_map<GlobalNodeID, Sensitivity>& aGradientMap, const std::vector<size_t>& aAllNodeIds)
    -> std::vector<double>
{
    // Build the gradient vector (3 entries for each node in the cut mesh) sorted by global node id
    constexpr size_t tNumDimensions{3};
    std::vector<double> tGradientVector(aAllNodeIds.size() * tNumDimensions);
    auto tGradientVectorMultiView = utilities::MultiVectorView(tGradientVector, tNumDimensions);
    for (size_t tNodeIndex = 0; tNodeIndex < aAllNodeIds.size(); ++tNodeIndex)
    {
        const auto tGlobalNodeID = aAllNodeIds[tNodeIndex];
        if (aGradientMap.find(tGlobalNodeID) != aGradientMap.end())
        {
            tGradientVectorMultiView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{0}) =
                aGradientMap.at(tGlobalNodeID).x;
            tGradientVectorMultiView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{1}) =
                aGradientMap.at(tGlobalNodeID).y;
            tGradientVectorMultiView(utilities::VectorIndex{tNodeIndex}, utilities::ComponentIndex{2}) =
                aGradientMap.at(tGlobalNodeID).z;
        }
    }
    return tGradientVector;
}

[[nodiscard]] double exponential_step_function(const double aInput)
{
    return aInput > 0.0 ? std::exp(-1.0 / aInput) : 0.0;
}

[[nodiscard]] double d_exponential_step_function(const double aInput)
{
    return aInput > 0.0 ? std::exp(-1.0 / aInput) / (aInput * aInput) : 0.0;
}

[[nodiscard]] double smoothing_function(const double aInput)
{
    return exponential_step_function(aInput) /
           (exponential_step_function(aInput) + exponential_step_function(1.0 - aInput));
}

[[nodiscard]] double d_smoothing_function(const double aInput)
{
    return (d_exponential_step_function(aInput) * exponential_step_function(1.0 - aInput) +
            d_exponential_step_function(1.0 - aInput) * exponential_step_function(aInput)) /
           std::pow((exponential_step_function(aInput) + exponential_step_function(1.0 - aInput)), 2);
}

[[nodiscard]] double overhang_value_from_normal_and_build_direction(const double aAngleDotBuildDirection,
                                                                    const OverhangCriterion& aOverhangCriterion)
{
    // The calculation of the return value will be determined by whether the input value, aAngleDotBuildDirection,
    // is in one of three ranges: 1) less than the step function transition region (return 1.0), 2) inside the step
    // function transiion region (calculate transition value), or 3) to the right of the transition region (return 0.0).
    double tReturnValue = 0.0;
    if (aAngleDotBuildDirection <= (aOverhangCriterion.mOverhangAngleThreshold - aOverhangCriterion.mTransitionWidth))
    {
        tReturnValue = 1.0;
    }
    else if (aAngleDotBuildDirection < aOverhangCriterion.mOverhangAngleThreshold)
    {
        tReturnValue = smoothing_function((aOverhangCriterion.mOverhangAngleThreshold - aAngleDotBuildDirection) /
                                          aOverhangCriterion.mTransitionWidth);
    }
    return tReturnValue;
}

[[nodiscard]] double d_overhang_value_from_normal_and_build_direction(const double aAngleDotBuildDirection,
                                                                      const OverhangCriterion& aOverhangCriterion)
{
    // The calculation of the return value will be determined by whether the input value, aAngleDotBuildDirection,
    // is in one of three ranges: 1) less than the step function transition region (return 1.0), 2) inside the step
    // function transiion region (calculate transition value), or 3) to the right of the transition region (return 0.0).
    double tReturnValue = 0.0;
    if (aAngleDotBuildDirection <= (aOverhangCriterion.mOverhangAngleThreshold - aOverhangCriterion.mTransitionWidth))
    {
        tReturnValue = 0.0;
    }
    else if (aAngleDotBuildDirection < aOverhangCriterion.mOverhangAngleThreshold)
    {
        tReturnValue = -d_smoothing_function((aOverhangCriterion.mOverhangAngleThreshold - aAngleDotBuildDirection) /
                                             aOverhangCriterion.mTransitionWidth) /
                       aOverhangCriterion.mTransitionWidth;
    }
    return tReturnValue;
}

[[nodiscard]] double overhang_from_triangle(const SensitivityTriangle& aTriangle,
                                            const OverhangCriterion& aOverhangCriterion)
{
    const Vector3 tNormal = aTriangle.normal();
    const double tNormalDotBuildDirection = dot(tNormal, aOverhangCriterion.mBuildDirection);
    return overhang_value_from_normal_and_build_direction(tNormalDotBuildDirection, aOverhangCriterion);
}

[[nodiscard]] double d_overhang_from_triangle(const SensitivityTriangle& aTriangle,
                                              const OverhangCriterion& aOverhangCriterion)
{
    const Vector3 tNormal = aTriangle.normal();
    const double tNormalDotBuildDirection = dot(tNormal, aOverhangCriterion.mBuildDirection);
    return d_overhang_value_from_normal_and_build_direction(tNormalDotBuildDirection, aOverhangCriterion);
}

[[nodiscard]] double area_weighted_overhang_from_triangle(const SensitivityTriangle& aTriangle,
                                                          const OverhangCriterion& aOverhangCriterion)
{
    const double tArea = aTriangle.area();
    return tArea * overhang_from_triangle(aTriangle, aOverhangCriterion);
}

TriangleGradient get_gradient_contribution_for_triangle(const SensitivityTriangle& aTriangle,
                                                        const OverhangCriterion& aOverhangCriterion)
{
    namespace tpik = plato::third_party_integration::krino;

    constexpr size_t tNumNodesPerTriangle{3};
    const double tArea = aTriangle.area();
    const double tOverhangPrime = d_overhang_from_triangle(aTriangle, aOverhangCriterion);
    const Vector3 tScaledBuildDir = aOverhangCriterion.mBuildDirection * tOverhangPrime * tArea;
    const double tOverhang = overhang_from_triangle(aTriangle, aOverhangCriterion);

    const auto tNormalSensitivities = tpik::get_d_normal_d_tri_node(aTriangle);
    const auto tAreaSensitivities = tpik::get_d_area_d_tri_node(aTriangle);

    TriangleGradient tGradient{NodeGradient{aTriangle.mNodes[0].first, {.x = 0., .y = 0., .z = 0.}},
                               NodeGradient{aTriangle.mNodes[1].first, {.x = 0., .y = 0., .z = 0.}},
                               NodeGradient{aTriangle.mNodes[2].first, {.x = 0., .y = 0., .z = 0.}}};
    // Loop over nodes in triangle and add contributions to derivative map
    for (size_t tNodeIndex = 0; tNodeIndex < tNumNodesPerTriangle; tNodeIndex++)
    {
        const Sensitivity tNormalSensitivity{
            third_party_integration::common::dot(tScaledBuildDir,
                                                 tNormalSensitivities[tNodeIndex][kDNormalDNodeXCoord]),
            third_party_integration::common::dot(tScaledBuildDir,
                                                 tNormalSensitivities[tNodeIndex][kDNormalDNodeYCoord]),
            third_party_integration::common::dot(tScaledBuildDir,
                                                 tNormalSensitivities[tNodeIndex][kDNormalDNodeZCoord])};
        tGradient[tNodeIndex].second += (tAreaSensitivities[tNodeIndex] * tOverhang + tNormalSensitivity);
    }
    return tGradient;
}

input_parser::overhang_criterion parse_input_deck(const std::string& aFilename)
{
    const auto tExists = std::filesystem::exists(aFilename);
    if (!tExists)
    {
        throw utilities::Exception{"Couldn't find overhang criterion input deck " + aFilename + "."};
    }
    auto tInputStream = std::ifstream{aFilename};
    const auto tInputFileString =
        std::string((std::istreambuf_iterator<char>(tInputStream)), std::istreambuf_iterator<char>());

    const auto tParser = input_parser::BlockStructRule<std::string::const_iterator, input_parser::overhang_criterion>{};

    auto tIter = tInputFileString.begin();
    auto tData = input_parser::overhang_criterion{};
    const auto tSkipper = input_parser::SkipperRule<std::string::const_iterator>{};
    const auto tParseSucceeded =
        phrase_parse(tIter, tInputFileString.cend(), tParser.mBlockRule, tSkipper.skipperRule(), tData);

    if (!tParseSucceeded || tIter != tInputFileString.cend())
    {
        throw utilities::Exception{"Couldn't parse overhang criterion input deck " + aFilename + "."};
    }
    return tData;
}

}  // namespace detail

}  // namespace plato::criteria::extension
