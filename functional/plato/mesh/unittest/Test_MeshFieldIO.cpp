#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

namespace plato::mesh::unittest
{
namespace
{
const auto kOutputMeshPath = std::filesystem::path{"out.exo"};
constexpr auto kFieldName = std::string_view{"Topology"};
constexpr auto kFixedValue = -1.0;

class MeshFieldWriterTestMesh : public plato::third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh
{
};

template <typename MeshIO>
void add_element_field(MeshIO& aMeshIO,
                       const ElementFieldVectorReference& aScalarField,
                       std::string_view aFieldName,
                       double aFixedValue)
{
    const auto tDesignVariables = DesignVariablesConversion{aMeshIO}.elementFieldToAnalysisDomainMesh(aScalarField);
    aMeshIO.addFieldFromAnalysisDomainMesh(tDesignVariables, aFieldName, aFixedValue);
}

template <typename MeshIO>
void add_nodal_field(MeshIO& aMeshIO,
                     const NodalFieldVectorReference& aScalarField,
                     std::string_view aFieldName,
                     double aFixedValue)
{
    const auto tDesignVariables = DesignVariablesConversion{aMeshIO}.nodalFieldToAnalysisDomainMesh(aScalarField);
    aMeshIO.addFieldFromAnalysisDomainMesh(tDesignVariables, aFieldName, aFixedValue);
}

void check_read_write_nodal_round_trip(const std::vector<double>& aFieldToWrite,
                                       const Mesh& aMesh,
                                       const std::vector<double>& aExpectedField,
                                       const test_utilities::TestContext& aTestContext)
{
    {
        MeshFieldWriter tWriter{aMesh, kOutputMeshPath};
        add_nodal_field(tWriter, NodalFieldVectorReference{aFieldToWrite}, kFieldName, kFixedValue);
    }
    const auto tFieldFromDisk =
        third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(kOutputMeshPath, kFieldName);
    EXPECT_EQ(tFieldFromDisk, aExpectedField) << aTestContext;
    std::filesystem::remove(kOutputMeshPath);
}

void check_read_write_element_round_trip(const std::vector<double>& aFieldToWrite,
                                         const Mesh& aMesh,
                                         const std::vector<double>& aExpectedField,
                                         const test_utilities::TestContext& aTestContext)
{
    {
        MeshFieldWriter tWriter{aMesh, kOutputMeshPath};
        add_element_field(tWriter, ElementFieldVectorReference{aFieldToWrite}, kFieldName, kFixedValue);
    }
    const auto tFieldFromDisk =
        third_party_integration::stk_io::test_utilities::read_element_field_as_vector(kOutputMeshPath, kFieldName);
    EXPECT_EQ(tFieldFromDisk, aExpectedField) << aTestContext;
    std::filesystem::remove(kOutputMeshPath);
}

}  // namespace

TEST_F(MeshFieldWriterTestMesh, WriteNodalFieldNoFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tField = std::vector<double>(mExpectedNumberOfNodes);
    std::iota(tField.begin(), tField.end(), 1.0);
    check_read_write_nodal_round_trip(tField, tMesh, tField, TEST_CONTEXT("Nodal field, no fixed blocks"));
}

TEST_F(MeshFieldWriterTestMesh, WriteNodalFieldOneFixedBlock)
{
    const auto tFixedBlock = std::set<std::string>{mBlockNames[0]};
    const auto tMesh = Mesh{mMeshFilePath, tFixedBlock};
    const auto tFieldToWrite = std::vector{1.0, 2.0, 3.0, 4.0};
    const auto tExpectedField =
        std::vector{kFixedValue, tFieldToWrite[0], tFieldToWrite[1], tFieldToWrite[2], tFieldToWrite[3], kFixedValue};
    check_read_write_nodal_round_trip(tFieldToWrite, tMesh, tExpectedField,
                                      TEST_CONTEXT("Nodal field, one fixed block"));
}

TEST_F(MeshFieldWriterTestMesh, WriteElementFieldNoFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tField = std::vector<double>(mExpectedNumberOfElements);
    std::iota(tField.begin(), tField.end(), 1.0);
    check_read_write_element_round_trip(tField, tMesh, tField, TEST_CONTEXT("Element field, no fixed blocks"));
}

TEST_F(MeshFieldWriterTestMesh, WriteElementFieldOneFixedBlock)
{
    const auto tFixedBlock = std::set<std::string>{mBlockNames[0]};
    const auto tMesh = Mesh{mMeshFilePath, tFixedBlock};
    const auto tFieldToWrite = std::vector{1.0, 2.0};
    const auto tExpectedField = std::vector{tFieldToWrite[0], tFieldToWrite[1], kFixedValue};
    check_read_write_element_round_trip(tFieldToWrite, tMesh, tExpectedField,
                                        TEST_CONTEXT("Element field, one fixed block"));
}

TEST_F(MeshFieldWriterTestMesh, WriteFieldsFromDesignVariables)
{
    const auto tWriteFieldToDisk = [this](const std::vector<analysis::ScalarFieldValue>& aFieldVector1,
                                          const std::vector<analysis::ScalarFieldValue>& aFieldVector2,
                                          const std::string_view aFieldName)
    {
        const auto tBlockField = analysis::AnalysisDomainMesh::BlockScalarField{{1, aFieldVector1}, {2, aFieldVector2}};
        const auto tDesignVariables = analysis::AnalysisDomainMesh{mMeshFilePath, tBlockField};
        auto tMesh = MeshFieldWriter{Mesh{mMeshFilePath}, kOutputMeshPath};
        tMesh.addFieldFromAnalysisDomainMesh(tDesignVariables, aFieldName, kFixedValue);
    };

    // Nodal field
    {
        constexpr auto tFieldName = std::string_view{"nodal_topology"};
        const auto tFieldVectorBlock1 =
            std::vector<analysis::ScalarFieldValue>{{1, 0, 1.0}, {2, 1, 2.0}, {5, 4, 5.0}, {6, 5, 6.0}};
        const auto tFieldVectorBlock2 =
            std::vector<analysis::ScalarFieldValue>{{2, 1, 2.0}, {3, 2, 3.0}, {4, 3, 4.0}, {5, 6, 5.0}};
        tWriteFieldToDisk(tFieldVectorBlock1, tFieldVectorBlock2, tFieldName);
        const auto tFieldFromDisk =
            third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(kOutputMeshPath, tFieldName);
        auto tExpectedField = std::vector<double>(mExpectedNumberOfNodes);
        std::iota(tExpectedField.begin(), tExpectedField.end(), 1.0);
        EXPECT_EQ(tFieldFromDisk, tExpectedField);
    }
    // Element field
    {
        constexpr auto tFieldName = std::string_view{"element_topology"};
        const auto tFieldVectorBlock1 = std::vector<analysis::ScalarFieldValue>{{3, 2, 3.0}};
        const auto tFieldVectorBlock2 = std::vector<analysis::ScalarFieldValue>{{1, 0, 1.0}, {2, 1, 2.0}};
        tWriteFieldToDisk(tFieldVectorBlock1, tFieldVectorBlock2, tFieldName);
        const auto tFieldFromDisk =
            third_party_integration::stk_io::test_utilities::read_element_field_as_vector(kOutputMeshPath, tFieldName);
        auto tExpectedField = std::vector<double>(mExpectedNumberOfElements);
        std::iota(tExpectedField.begin(), tExpectedField.end(), 1.0);
        EXPECT_EQ(tFieldFromDisk, tExpectedField);
    }
    std::filesystem::remove(kOutputMeshPath);
}

TEST_F(MeshFieldWriterTestMesh, WriteTwoFields)
{
    constexpr auto tElementFieldName = std::string_view{"elmers_glue"};
    constexpr auto tNodalFieldName = std::string_view{"nodule"};

    auto tElementField = std::vector<double>(mExpectedNumberOfElements);
    std::iota(tElementField.begin(), tElementField.end(), 1.0);

    auto tNodalField = std::vector<double>(mExpectedNumberOfNodes);
    std::iota(tNodalField.begin(), tNodalField.end(), 1.0);
    std::reverse(tNodalField.begin(), tNodalField.end());

    {
        auto tMeshWriter = MeshFieldWriter{Mesh{mMeshFilePath}, kOutputMeshPath};
        add_element_field(tMeshWriter, ElementFieldVectorReference{tElementField}, tElementFieldName, kFixedValue);
        add_nodal_field(tMeshWriter, NodalFieldVectorReference{tNodalField}, tNodalFieldName, kFixedValue);
    }

    const auto tNodalFieldFromDisk =
        third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(kOutputMeshPath, tNodalFieldName);
    EXPECT_EQ(tNodalFieldFromDisk, tNodalField);

    const auto tElementFieldFromDisk = third_party_integration::stk_io::test_utilities::read_element_field_as_vector(
        kOutputMeshPath, tElementFieldName);
    EXPECT_EQ(tElementFieldFromDisk, tElementField);

    std::filesystem::remove(kOutputMeshPath);
}

TEST_F(MeshFieldWriterTestMesh, WriteFieldsForTwoTimeSteps)
{
    constexpr auto tElementFieldName = std::string_view{"barley"};
    constexpr auto tNodalFieldName = std::string_view{"corn"};

    // write first time step
    auto tFirstElementField = std::vector<double>(mExpectedNumberOfElements);
    std::fill(tFirstElementField.begin(), tFirstElementField.end(), 71.0);
    auto tFirstNodalField = std::vector<double>(mExpectedNumberOfNodes);
    std::iota(tFirstNodalField.begin(), tFirstNodalField.end(), 1.0);
    constexpr double tFirstTimeStep = 1.0;
    {
        auto tMeshWriter = MeshFieldWriter{Mesh{mMeshFilePath}, kOutputMeshPath, tFirstTimeStep};
        add_element_field(tMeshWriter, ElementFieldVectorReference{tFirstElementField}, tElementFieldName, kFixedValue);
        add_nodal_field(tMeshWriter, NodalFieldVectorReference{tFirstNodalField}, tNodalFieldName, kFixedValue);
    }

    // write second time step
    constexpr double tSecondTimeStep = 2.0;
    auto tSecondElementField = std::vector<double>(mExpectedNumberOfElements);
    std::fill(tSecondElementField.begin(), tSecondElementField.end(), 86.0);
    auto tSecondNodalField = std::vector<double>(mExpectedNumberOfNodes);
    std::iota(tSecondNodalField.begin(), tSecondNodalField.end(), 59.0);
    {
        auto tMeshAppender = MeshFieldAppender{Mesh{kOutputMeshPath}, tSecondTimeStep};
        add_element_field(tMeshAppender, ElementFieldVectorReference{tSecondElementField}, tElementFieldName,
                          kFixedValue);
        add_nodal_field(tMeshAppender, NodalFieldVectorReference{tSecondNodalField}, tNodalFieldName, kFixedValue);
    }

    // check first time step
    {
        const auto tElementFieldFromDisk =
            third_party_integration::stk_io::test_utilities::read_element_field_as_vector(
                kOutputMeshPath, tElementFieldName, tFirstTimeStep);
        EXPECT_EQ(tElementFieldFromDisk, tFirstElementField);

        const auto tNodalFieldFromDisk = third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(
            kOutputMeshPath, tNodalFieldName, tFirstTimeStep);
        EXPECT_EQ(tNodalFieldFromDisk, tFirstNodalField);
    }

    // check second time step
    {
        const auto tElementFieldFromDisk =
            third_party_integration::stk_io::test_utilities::read_element_field_as_vector(
                kOutputMeshPath, tElementFieldName, tSecondTimeStep);
        EXPECT_EQ(tElementFieldFromDisk, tSecondElementField);

        const auto tNodalFieldFromDisk = third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(
            kOutputMeshPath, tNodalFieldName, tSecondTimeStep);
        EXPECT_EQ(tNodalFieldFromDisk, tSecondNodalField);
    }

    std::filesystem::remove(kOutputMeshPath);
}

}  // namespace plato::mesh::unittest
