#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <numeric>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::parallel_unittest
{
using third_party_integration::krino::test_utilities::KrinoTestFixture;
namespace
{

const auto kBoxFilePath = utilities::data_file_path("box_3x4x7_tet4.cdf");

}  // namespace

TEST_F(KrinoTestFixture, KrinoWrapperParallel)
{
    const auto tCommunicator = boost::mpi::communicator{};
    std::cout << "Before HERE" << std::endl;
    ASSERT_TRUE(kBoxFilePath.has_value());

    std::cout << "HERE" << std::endl;
    const auto tMesh = mesh::Mesh{kBoxFilePath.value()};
    std::cout << __LINE__ << std::endl;
    const auto tNumberOfDesignNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
    std::cout << __LINE__ << std::endl;
    std::vector<double> tDesignVariable(tNumberOfDesignNodes);
    std::cout << __LINE__ << std::endl;
    std::iota(tDesignVariable.begin(), tDesignVariable.end(), -static_cast<double>(tNumberOfDesignNodes) / 2.0);
    std::cout << __LINE__ << std::endl;
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tDesignVariable});
    std::cout << __LINE__ << std::endl;
    [[maybe_unused]] const auto tKrinoWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);
    std::cout << __LINE__ << std::endl;

    const auto tSensitivity = tKrinoWrapper.sensitivities();
    EXPECT_EQ(tSensitivity.size(), 4U);

    const auto tRank = tCommunicator.rank();

    std::size_t tGoldSize;
    if (tRank == 0)
    {
        tGoldSize = tSensitivity.size();
    }
    boost::mpi::broadcast(tCommunicator, tGoldSize, 0);
    EXPECT_EQ(tGoldSize, tSensitivity.size());

    tKrinoWrapper.writeCutMesh("out.exo", third_party_integration::krino::VoidPhase::kExcludeFromMesh);
}

}  // namespace plato::geometry::extension::parallel_unittest
