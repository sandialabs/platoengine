#include "plato/criteria/extension/OverhangCriterion.hpp"

#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshSidesets.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"
#include "plato/utilities/Zip.hpp"
// #include "plato/utilities/FixedWidthFloatingPointOutput.hpp"
// #include "plato/services/SystemLogger.hpp"

namespace plato::criteria::extension
{
namespace
{
// constexpr auto kCriterionValuePrecision = 8U;
// constexpr auto kCriterionValueFieldWidth = kCriterionValuePrecision + 1U;
constexpr auto kTransitionWidth{.1};
constexpr plato::third_party_integration::common::Vector3 kBuildDirection{0.0, 0.0, 1.0};
const auto kOverhangAngleThreshold = -std::sqrt(2.0) / 2.0;

using Registration =
    library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;

[[maybe_unused]] static auto kOverhangRegistration =
    Registration{library::builtin_criterion_registration_name(OverhangCriterion::kCriterionName),
                 [](const library::CriterionInput&) { return make_overhang_function(); }};
}  // namespace

double OverhangCriterion::f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    namespace tpistk = plato::third_party_integration::stk_io;

    //    auto tLogger = services::component_logger(mComponentType, mName);
    //    tLogger.logInfo("Evaluating criterion");

    const auto tMesh = mesh::MeshSidesets{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const std::vector<tpistk::Triangle> tTriangles = tMesh.sidesetTriangles("surface__void");
    const double tReturnValue = std::accumulate(tTriangles.begin(), tTriangles.end(), 0.0,
                                                [](double aCurrentSum, tpistk::Triangle aCurTri)
                                                {
                                                    return aCurrentSum + detail::area_weighted_overhang_from_triangle(
                                                                             aCurTri, kOverhangAngleThreshold,
                                                                             kTransitionWidth, kBuildDirection);
                                                });
    std::cout << "Overhang criterion value: " << tReturnValue << std::endl;
    /*
        tLogger.logInfo(
            "Evaluation complete. Criterion value = " +
            utilities::to_string(
                utilities::FixedWidthFloatingPointOutput<double, kCriterionValuePrecision, kCriterionValueFieldWidth>{
                    tValue}));
                    */
    return tReturnValue;
}

linear_algebra::DynamicVector<double> OverhangCriterion::df(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    namespace tpistk = plato::third_party_integration::stk_io;
    namespace tpik = plato::third_party_integration::krino;

    constexpr size_t tNumNodesPerTriangle{3};
    constexpr size_t tNumSpatialDimensions{3};

    const auto tSidesetMesh = mesh::MeshSidesets{mesh::Mesh{aAnalysisDomainMesh.mFileName}};

    const std::vector<tpistk::Triangle> tTriangles = tSidesetMesh.sidesetTriangles("surface__void");
    /*
std::ofstream outFile("output.txt");
outFile << "Num triangles: " << tTriangles.size() << std::endl;
outFile << "Triangle global ids:" << std::endl;
std::vector<size_t> tAllNodesInTris;
for(const auto &tCurTri : tTriangles)
{
    for(size_t i=0; i<3; i++)
    {
        outFile << tCurTri.global_ids[i] << " ";
        tAllNodesInTris.push_back(tCurTri.global_ids[i]);
    }
    outFile << std::endl;
}
outFile << "Total number of nodes in tris before uniquifying: " << tAllNodesInTris.size() << std::endl;
std::sort(tAllNodesInTris.begin(), tAllNodesInTris.end());
auto it = std::unique(tAllNodesInTris.begin(), tAllNodesInTris.end());
tAllNodesInTris.erase(it, tAllNodesInTris.end());
outFile << "Total number of nodes in tris sorting and uniquifying: " << tAllNodesInTris.size() << std::endl;
*/
    // size_t tCounter=0;
    //  Initialize all future map entries to 0.0
    std::map<size_t, std::array<double, 3>> tGradientMap;
    for (const auto& tCurTriangle : tTriangles)
    {
        for (const auto& tGlobalNodeID : tCurTriangle.global_ids)
        {
            tGradientMap[tGlobalNodeID] = {0.0, 0.0, 0.0};
            //           tCounter++;
        }
    }
    // outFile << "Total times gradient map was initialized (should match total # of nodes before uniquifying): " <<
    // tCounter << std::endl;
    //  Accumulate gradient contributions from all triangles
    for (const auto& tCurTriangle : tTriangles)
    {
        std::vector<double> tCurTriGradient = detail::get_gradient_contribution_for_triangle(
            tCurTriangle, kOverhangAngleThreshold, kTransitionWidth, kBuildDirection);
        for (size_t tNodeIndex = 0; tNodeIndex < tNumNodesPerTriangle; tNodeIndex++)
        {
            const size_t tCurGlobalNodeID = tCurTriangle.global_ids[tNodeIndex];
            /*
            if(tCurGlobalNodeID == 5772)
            {
                std::cout << "Tri with nodes: " << tCurTriangle.global_ids[0] << " " << tCurTriangle.global_ids[1] << "
            " << tCurTriangle.global_ids[2] << " contributing to node 5772. Grad: " <<
            tCurTriGradient[tNodeIndex*tNumSpatialDimensions]
                    << " " << tCurTriGradient[tNodeIndex*tNumSpatialDimensions+1] << " " <<
            tCurTriGradient[tNodeIndex*tNumSpatialDimensions+2] << std::endl;
            }
                    */
            for (size_t tSpatialIndex = 0; tSpatialIndex < tNumSpatialDimensions; tSpatialIndex++)
            {
                tGradientMap[tCurGlobalNodeID][tSpatialIndex] +=
                    tCurTriGradient[tNodeIndex * tNumSpatialDimensions + tSpatialIndex];
            }
        }
    }
    //    std::cout << "Total grad for Node 5772: " << tGradientMap[5772][0] << " " << tGradientMap[5772][1] << " " <<
    //    tGradientMap[5772][2] << std::endl;
    std::ofstream outJournal("out.jou");
    // outFile << "Gradient Map:" << std::endl;
    std::map<size_t, std::array<double, 3>>::iterator tIter = tGradientMap.begin();
    while (tIter != tGradientMap.end())
    {
        //    outFile << "NodeID: " << tIter->first << "; Gradient: " << tIter->second[0] << " " << tIter->second[1] <<
        //    " " << tIter->second[2] << std::endl;
        outJournal << "create vertex {Nx(" << tIter->first << ")} {Ny(" << tIter->first << ")} {Nz(" << tIter->first
                   << ")}" << std::endl;
        outJournal << "create vertex {Nx(" << tIter->first << ")+1*(" << tIter->second[0] << ")} {Ny(" << tIter->first
                   << ")+1*(" << tIter->second[1] << ")} {Nz(" << tIter->first << ")+1*(" << tIter->second[2] << ")}"
                   << std::endl;
        outJournal << "create curve vertex {Id('vertex')-1} {Id('vertex')}" << std::endl;
        tIter++;
    }
    outJournal.close();
    exit(0);

    // Build the gradient vector (3 entries for each node in the cut mesh) sorted by global node id
    const auto tEntityRetrievalMesh = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    std::vector<size_t> tAllNodeIds = tEntityRetrievalMesh.globalNodeIDs();
    std::sort(tAllNodeIds.begin(), tAllNodeIds.end());
    std::vector<double> tGradientVector(tAllNodeIds.size() * 3);
    size_t tIndex = 0;
    for (const auto& tCurNode : tAllNodeIds)
    {
        if (tGradientMap.find(tCurNode) == tGradientMap.end())
        {
            for (size_t i = 0; i < 3; ++i)
            {
                tGradientVector[tIndex + i] = 0.0;
            }
            tIndex += 3;
        }
        else
        {
            for (size_t i = 0; i < 3; ++i)
            {
                tGradientVector[tIndex + i] = tGradientMap[tCurNode][i];
            }
            tIndex += 3;
        }
    }
    /*
tCounter=0;
outFile << "Gradient Vector:" << std::endl;
for(const auto &tCurNode : tAllNodeIds)
{

    outFile << "NodeID: " << tCurNode << "; Gradient: " << tGradientVector[tCounter] << " " <<
tGradientVector[tCounter+1] << " " << tGradientVector[tCounter+2] << std::endl; tCounter += 3;
}
outFile.close();
*/

    const double tGradientNorm =
        std::sqrt(std::inner_product(tGradientVector.begin(), tGradientVector.end(), tGradientVector.begin(), 0));
    std::cout << "Overhang criterion gradient norm: " << tGradientNorm << std::endl;
    exit(0);
    return linear_algebra::DynamicVector<double>(std::move(tGradientVector));
}

auto make_overhang_function() -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [](const analysis::AnalysisDomainMesh& mesh) { return OverhangCriterion{}.f(mesh); },
        [](const analysis::AnalysisDomainMesh& mesh) { return OverhangCriterion{}.df(mesh); });
}

namespace detail
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::stk_io;

[[nodiscard]] double exponential_step_function(const double& aInput)
{
    return aInput > 0.0 ? std::exp(-1.0 / aInput) : 0.0;
}

[[nodiscard]] double d_exponential_step_function(const double& aInput)
{
    return aInput > 0.0 ? std::exp(-1.0 / aInput) / (aInput * aInput) : 0.0;
}

[[nodiscard]] double smoothing_function(const double& aInput)
{
    return exponential_step_function(aInput) /
           (exponential_step_function(aInput) + exponential_step_function(1.0 - aInput));
}

[[nodiscard]] double d_smoothing_function(const double& aInput)
{
    return (d_exponential_step_function(aInput) * exponential_step_function(1.0 - aInput) +
            d_exponential_step_function(1.0 - aInput) * exponential_step_function(aInput)) /
           std::pow((exponential_step_function(aInput) + exponential_step_function(1.0 - aInput)), 2);
}

[[nodiscard]] double overhang(const double& aAngleDotBuildDirection,
                              const double& aOverhangAngleThreshold,
                              const double& aStepTransitionWidth)
{
    // The calculation of the return value will be determined by whether the input value, aAngleDotBuildDirection,
    // is in one of three ranges: 1) less than the step function transition region (return 1.0), 2) inside the step
    // function transiion region (calculate transition value), or 3) to the right of the transition region (return 0.0).
    double tReturnValue = 0.0;
    if (aAngleDotBuildDirection <= (aOverhangAngleThreshold - aStepTransitionWidth))
    {
        tReturnValue = 1.0;
    }
    else if (aAngleDotBuildDirection < aOverhangAngleThreshold)
    {
        tReturnValue = smoothing_function((aOverhangAngleThreshold - aAngleDotBuildDirection) / aStepTransitionWidth);
    }
    return tReturnValue;
}

[[nodiscard]] double d_overhang(const double& aAngleDotBuildDirection,
                                const double& aOverhangAngleThreshold,
                                const double& aStepTransitionWidth)
{
    // The calculation of the return value will be determined by whether the input value, aAngleDotBuildDirection,
    // is in one of three ranges: 1) less than the step function transition region (return 1.0), 2) inside the step
    // function transiion region (calculate transition value), or 3) to the right of the transition region (return 0.0).
    double tReturnValue = 0.0;
    if (aAngleDotBuildDirection <= (aOverhangAngleThreshold - aStepTransitionWidth))
    {
        tReturnValue = 0.0;
    }
    else if (aAngleDotBuildDirection < aOverhangAngleThreshold)
    {
        tReturnValue =
            -d_smoothing_function((aOverhangAngleThreshold - aAngleDotBuildDirection) / aStepTransitionWidth) /
            aStepTransitionWidth;
    }
    return tReturnValue;
}

[[nodiscard]] double overhang_from_triangle_node_coordinates(const Coordinate& aNode1,
                                                             const Coordinate& aNode2,
                                                             const Coordinate& aNode3,
                                                             const double& aOverhangAngleThreshold,
                                                             const double& aStepTransitionWidth,
                                                             const Vector3& aBuildDirection)
{
    const Triangle tTriangle{aNode1, aNode2, aNode3};
    const Vector3 tNormal = tTriangle.normal();
    const double tNormalDotBuildDirection = dot(tNormal, aBuildDirection);
    return overhang(tNormalDotBuildDirection, aOverhangAngleThreshold, aStepTransitionWidth);
}

[[nodiscard]] double d_overhang_from_triangle_node_coordinates(const Coordinate& aNode1,
                                                               const Coordinate& aNode2,
                                                               const Coordinate& aNode3,
                                                               const double& aOverhangAngleThreshold,
                                                               const double& aStepTransitionWidth,
                                                               const Vector3& aBuildDirection)
{
    const Triangle tTriangle{aNode1, aNode2, aNode3};
    const Vector3 tNormal = tTriangle.normal();
    const double tNormalDotBuildDirection = dot(tNormal, aBuildDirection);
    return d_overhang(tNormalDotBuildDirection, aOverhangAngleThreshold, aStepTransitionWidth);
}

[[nodiscard]] double area_weighted_overhang_from_triangle(const Triangle& aTriangle,
                                                          const double& aOverhangAngleThreshold,
                                                          const double& aStepTransitionWidth,
                                                          const Vector3& aBuildDirection)
{
    const double tArea = aTriangle.volume();
    return tArea * overhang_from_triangle_node_coordinates(aTriangle.p0, aTriangle.p1, aTriangle.p2,
                                                           aOverhangAngleThreshold, aStepTransitionWidth,
                                                           aBuildDirection);
}

void gradient_contribution_for_triangle(
    const Triangle& aTriangle,
    const double& aOverhangAngleThreshold,
    const double& aStepTransitionWidth,
    const Vector3& aBuildDirection,
    third_party_integration::krino::TriangleSensitivity& tAreaAndNormalSensitivities,
    std::map<size_t, std::array<double, 3>>& aGradientMap)
{
    constexpr auto tNumDimensions{3};
    const double tArea = aTriangle.volume();
    const double tOverhangPrime = d_overhang_from_triangle_node_coordinates(
        aTriangle.p0, aTriangle.p1, aTriangle.p2, aOverhangAngleThreshold, aStepTransitionWidth, aBuildDirection);
    const Vector3 tScaledBuildDir = aBuildDirection * tOverhangPrime * tArea;
    const double tOverhang = overhang_from_triangle_node_coordinates(
        aTriangle.p0, aTriangle.p1, aTriangle.p2, aOverhangAngleThreshold, aStepTransitionWidth, aBuildDirection);
    // Loop over nodes in triangle and add contributions to derivative map
    for (const auto& tCurNodeGlobalId : aTriangle.global_ids)
    {
        // Initialize derivate map entry if first time for this node
        if (aGradientMap.find(tCurNodeGlobalId) == aGradientMap.end())
        {
            aGradientMap[tCurNodeGlobalId] = {0, 0, 0};
        }
        for (size_t iDimIndex = 0; iDimIndex < tNumDimensions; ++iDimIndex)
        {
            // Add the phi*dArea contribution
            aGradientMap[tCurNodeGlobalId][iDimIndex] +=
                tOverhang * tAreaAndNormalSensitivities.areaSensitivities[tCurNodeGlobalId][iDimIndex];
            // Add the phi_prime*dNormal*build_direction*Area contribution
            Vector3 tNormalSensitivity{tAreaAndNormalSensitivities.normalSensitivities[tCurNodeGlobalId][iDimIndex][0],
                                       tAreaAndNormalSensitivities.normalSensitivities[tCurNodeGlobalId][iDimIndex][1],
                                       tAreaAndNormalSensitivities.normalSensitivities[tCurNodeGlobalId][iDimIndex][2]};
            aGradientMap[tCurNodeGlobalId][iDimIndex] +=
                third_party_integration::common::dot(tScaledBuildDir, tNormalSensitivity);
        }
    }
}

std::vector<double> get_gradient_contribution_for_triangle(const Triangle& aTriangle,
                                                           const double& aOverhangAngleThreshold,
                                                           const double& aStepTransitionWidth,
                                                           const Vector3& aBuildDirection)
{
    namespace tpik = plato::third_party_integration::krino;

    constexpr size_t tNumNodesPerTriangle{3};
    constexpr auto tNumDimensions{3};
    std::vector<double> tGradient(tNumNodesPerTriangle * tNumDimensions, 0.0);
    const double tArea = aTriangle.volume();
    const double tOverhangPrime = d_overhang_from_triangle_node_coordinates(
        aTriangle.p0, aTriangle.p1, aTriangle.p2, aOverhangAngleThreshold, aStepTransitionWidth, aBuildDirection);
    const Vector3 tScaledBuildDir = aBuildDirection * tOverhangPrime * tArea;
    const double tOverhang = overhang_from_triangle_node_coordinates(
        aTriangle.p0, aTriangle.p1, aTriangle.p2, aOverhangAngleThreshold, aStepTransitionWidth, aBuildDirection);

    tpik::TriangleNormalSensitivity tTriangleNormalSensitivity = tpik::get_d_normal_d_nodal_coords_from_tri(aTriangle);
    tpik::TriangleAreaSensitivity tTriangleAreaSensitivity = tpik::get_d_area_d_nodal_coords_from_tri(aTriangle);

    // Loop over nodes in triangle and add contributions to derivative map
    for (size_t tNodeIndex = 0; tNodeIndex < tNumNodesPerTriangle; tNodeIndex++)
    {
        const auto tGlobalNodeId = aTriangle.global_ids[tNodeIndex];
        for (size_t tDimIndex = 0; tDimIndex < tNumDimensions; ++tDimIndex)
        {
            // Add the phi*dArea contribution
            tGradient[tNumDimensions * tNodeIndex + tDimIndex] +=
                tOverhang * tTriangleAreaSensitivity[tGlobalNodeId][tDimIndex];
            // Add the phi_prime*dNormal*build_direction*Area contribution
            Vector3 tNormalSensitivity{tTriangleNormalSensitivity[tGlobalNodeId][tDimIndex][0],
                                       tTriangleNormalSensitivity[tGlobalNodeId][tDimIndex][1],
                                       tTriangleNormalSensitivity[tGlobalNodeId][tDimIndex][2]};
            tGradient[tNumDimensions * tNodeIndex + tDimIndex] +=
                third_party_integration::common::dot(tScaledBuildDir, tNormalSensitivity);
        }
    }
    return tGradient;
}

}  // namespace detail

}  // namespace plato::criteria::extension
