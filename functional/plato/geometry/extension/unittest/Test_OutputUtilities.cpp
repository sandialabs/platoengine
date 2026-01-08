#include <gtest/gtest.h>

#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kTestOutputPath = std::filesystem::path{"test-result.exo"};

class OutputUtilitiesTest : public third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh
{
   public:
    [[nodiscard]] auto solution_vector() -> linear_algebra::DynamicVector<double>
    {
        constexpr auto tFieldValue = 0.25;
        return linear_algebra::DynamicVector<double>(mCommandGenerator.numberOfNodes(), tFieldValue);
    }
};

auto analysis_domain_mesh_with_constant_entries(analysis::AnalysisDomainMesh aAnalysisDomainMesh, const double aValue)
{
    for (auto& [tBlockID, tBlockEntries] : aAnalysisDomainMesh.mBlockScalarField)
    {
        std::transform(tBlockEntries.cbegin(), tBlockEntries.cend(), tBlockEntries.begin(),
                       [aValue](const auto& aEntry) {
                           return analysis::ScalarFieldValue{aEntry.mGlobalMeshEntityID,
                                                             aEntry.mDesignVariableVectorIndex, aValue};
                       });
    }
    return aAnalysisDomainMesh;
}

/// @brief Sets all entries in the mesh to a constant value.
auto constant_value_filter_function(const double aValue) -> filter::library::FilterFunction
{
    return filter::library::FilterFunction{
        [aValue](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return analysis_domain_mesh_with_constant_entries(aAnalysisDomainMesh, aValue); },
        [](const analysis::AnalysisDomainMesh&) { return filter::library::FilterJacobian{}; },
        [](const analysis::AnalysisDomainMesh&)
        { return filter::library::FilterAdjointJacobian{filter::library::FilterJacobian{}}; }};
}

constexpr auto kFixedValue = 1.0;
constexpr auto kControlFieldName = std::string_view{"control"};
constexpr auto kFilteredFieldName = std::string_view{"filtered"};
}  // namespace

TEST_F(OutputUtilitiesTest, OutputNodalFieldOneBlockMeshFiltered)
{
    const auto tMeshOutputInfo = MeshFieldOutputInfo{mesh::Mesh{mMeshFilePath}, kTestOutputPath,    {},
                                                     kControlFieldName,         kFilteredFieldName, kFixedValue};
    const auto tSolution = solution_vector();
    constexpr auto tFilteredFieldValue = 0.5;
    const auto tFilteredMesh = output_nodal_field(tMeshOutputInfo, constant_value_filter_function(tFilteredFieldValue),
                                                  tSolution, output::kOverwriteInfo);

    const auto tReadControlField =
        third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(kTestOutputPath, kControlFieldName);
    EXPECT_EQ(tReadControlField, tSolution.stdVector());

    const auto tReadFilteredField = third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(
        kTestOutputPath, kFilteredFieldName);
    auto tExpectedFilteredField = std::vector<double>(mCommandGenerator.numberOfNodes());
    std::fill(tExpectedFilteredField.begin(), tExpectedFilteredField.end(), tFilteredFieldValue);
    EXPECT_EQ(tReadFilteredField, tExpectedFilteredField);

    EXPECT_EQ(tFilteredMesh.mBlockScalarField.at(1U).size(), mCommandGenerator.numberOfNodes());
    for (const auto& aScalarFieldValue : tFilteredMesh.mBlockScalarField.at(1U))
    {
        EXPECT_EQ(aScalarFieldValue.mValue, tFilteredFieldValue);
    }

    std::filesystem::remove(kTestOutputPath);
}

}  // namespace plato::geometry::extension::unittest
