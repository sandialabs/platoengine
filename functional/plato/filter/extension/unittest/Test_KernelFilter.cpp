#include <gtest/gtest.h>

#include <boost/math/constants/constants.hpp>
#include <boost/mpi/communicator.hpp>
#include <vector>

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::filter::extension::unittest
{

namespace
{

constexpr std::string_view kMeshFile = "mesh.exo";
constexpr double kTolerance = 1e-14;  // for comparison against matlab values

[[nodiscard]] std::pair<std::vector<double>, std::vector<double> > test_filter_evaluation(
    const input_parser::KernelFilterCenteringTypes aFilterCentering)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{{1, 1, 1}, {0, 0, 0}, {1, 1, 1}};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    const FilterRadius tFilterRadius{1.1};

    const KernelFilter tKernelFilter{mesh::Mesh{kMeshFile}, tFilterRadius, aFilterCentering,
                                     boost::mpi::communicator{}};

    std::vector<double> tNodalDensities(tCommandGenerator.numberOfNodes(), 0);
    const int tHalfNode = tNodalDensities.size() / 2;
    tNodalDensities[tHalfNode] = 1;
    tNodalDensities[tHalfNode - 1] = .5;
    tNodalDensities[tHalfNode + 1] = .5;

    const auto tMesh = mesh::DesignVariablesConversion{mesh::Mesh{kMeshFile}};
    const auto tMeshDesignVariables =
        tMesh.nodalDensitiesToMeshDesignVariables(mesh::NodalDensityVectorReference{tNodalDensities});

    const auto tResult = tKernelFilter.filter(tMeshDesignVariables);
    const auto [tPostFilter, tIDMap] = mesh::detail::split_densities(
        mesh::mesh_design_variables_to_vector(mesh::MeshDesignVariablesDensitiesView{tResult}));

    std::vector<double> tStdVectorSensitivities;
    if (aFilterCentering == input_parser::KernelFilterCenteringTypes::kElementCentered)
    {
        tStdVectorSensitivities = std::vector<double>(tCommandGenerator.numberOfElements(), 1);
    }
    else
    {
        tStdVectorSensitivities = std::vector<double>(tCommandGenerator.numberOfNodes(), 0);
        tStdVectorSensitivities[0] = 1;
        tStdVectorSensitivities[1] = .5;
        tStdVectorSensitivities[2] = .25;
    }

    const auto tPostSensitivities =
        tKernelFilter
            .jacobianTimesVector(tMeshDesignVariables, linear_algebra::DynamicVector<double>(tStdVectorSensitivities))
            .stdVector();

    test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));

    return {tPostFilter, tPostSensitivities};
}

}  // namespace

/*
    DistanceMatrix M = [1/8 1/8 1/8 1/8 1/8 1/8 1/8 1/8]
    incoming densities rho = [0 0 0 .5 1 .5 0 0]
    M*rho = 1/4

    incoming sensitivities v = [1 ]
    v*M =[1/8 1/8 1/8 1/8 1/8 1/8 1/8 1/8]
*/

TEST(KernelFilter, SingleHexElementCentered)
{
    const auto [tResultFilter, tResultJV] =
        test_filter_evaluation(input_parser::KernelFilterCenteringTypes::kElementCentered);

    ASSERT_EQ(tResultFilter.size(), 1u);
    EXPECT_NEAR(tResultFilter[0], 1.0 / 4.0, kTolerance);

    ASSERT_EQ(tResultJV.size(), 8u);
    for (unsigned int tIndex = 0; tIndex < 8; ++tIndex)
    {
        EXPECT_NEAR(tResultJV[tIndex], 1.0 / 8.0, kTolerance);
    }
}
// clang-format off

/*
    DistanceMatrixBefore Normalization = [1        .1/1.1   .1/1.1      0           .1/1.1      0           0            0      ;
                                          .1/1.1   1        0           .1/1.1      0           .1/1.1      0            0      ;
                                          .1/1.1   0        1           .1/1.1      0           0          .1/1.1       0      ;
                                          0        .1/1.1   .1/1.1      1           0           0           0            .1/1.1    ;
                                          .1/1.1   0        0           0           1           .1/1.1      .1/1.1       0    ;
                                          0        .1/1.1   0           0           .1/1.1      1           0            .1/1.1     ;
                                          0        0        .1/1.1      0           .1/1.1      0           1            .1/1.1     ;
                                          0        0        0           .1/1.1      0           .1/1.1      .1/1.1       1     ];
            ]
    incoming densities rho = [0 0 0 .5 1 .5 0 0]
    M*rho =[ 0.0714285714285714
        0.0714285714285714
        0.0357142857142857
         0.392857142857143
         0.821428571428571
         0.464285714285714
        0.0714285714285714
        0.0714285714285714 ]

    incoming sensitivities v = [1 .5 .25 0 0 0 0 0 ]
    v*M =0.839285714285714         0.464285714285714         0.267857142857143        0.0535714285714286        0.0714285714285714        0.0357142857142857        0.0178571428571429                         0
*/
// clang-format on
TEST(KernelFilter, SingleHexNodalCentered)
{
    const auto [tResultFilter, tResultJV] =
        test_filter_evaluation(input_parser::KernelFilterCenteringTypes::kNodeCentered);

    ASSERT_EQ(tResultFilter.size(), 8u);
    EXPECT_NEAR(tResultFilter[0], 0.0714285714285714, kTolerance);
    EXPECT_NEAR(tResultFilter[1], 0.0714285714285714, kTolerance);
    EXPECT_NEAR(tResultFilter[2], 0.0357142857142857, kTolerance);
    EXPECT_NEAR(tResultFilter[3], 0.392857142857143, kTolerance);
    EXPECT_NEAR(tResultFilter[4], 0.821428571428571, kTolerance);
    EXPECT_NEAR(tResultFilter[5], 0.464285714285714, kTolerance);
    EXPECT_NEAR(tResultFilter[6], 0.0714285714285714, kTolerance);
    EXPECT_NEAR(tResultFilter[7], 0.0714285714285714, kTolerance);

    ASSERT_EQ(tResultJV.size(), 8u);
    EXPECT_NEAR(tResultJV[0], 0.839285714285714, kTolerance);
    EXPECT_NEAR(tResultJV[1], 0.464285714285714, kTolerance);
    EXPECT_NEAR(tResultJV[2], 0.267857142857143, kTolerance);
    EXPECT_NEAR(tResultJV[3], 0.0535714285714286, kTolerance);
    EXPECT_NEAR(tResultJV[4], 0.0714285714285714, kTolerance);
    EXPECT_NEAR(tResultJV[5], 0.0357142857142857, kTolerance);
    EXPECT_NEAR(tResultJV[6], 0.0178571428571429, kTolerance);
    EXPECT_NEAR(tResultJV[7], 0, kTolerance);
}

TEST(KernelFilter, ProperlyAllocatesMemoryFor2DMesh)
{
    auto tInput = plato::test_utilities::create_valid_kernel_filter();
    tInput.filter_radius = 5e-1;
    auto tFilterCache = detail::create_filter_cache(tInput);

    const auto tFilePath = test_utilities::test_data_file_path("rectangle_3x4_tri3.cdf");
    ASSERT_TRUE(tFilePath.has_value());

    const auto tMesh = mesh::Mesh{tFilePath.value()};
    const auto tNodalDensities = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 1.0);
    const auto tMeshDesignVariables = mesh::DesignVariablesConversion{tMesh}.nodalDensitiesToMeshDesignVariables(
        mesh::NodalDensityVectorReference{tNodalDensities});

    ASSERT_NO_THROW([[maybe_unused]] const auto tFilter = tFilterCache.compute(tMeshDesignVariables));
}

TEST(KernelFilterDetail, CreateFilterCache_UseToApplyFilter)
{
    auto tFilterCache = detail::create_filter_cache(plato::test_utilities::create_valid_kernel_filter());

    // make mesh and filter
    {
        const auto tCommandGenerator =
            third_party_integration::stk_io::CommandGenerator{{2, 2, 2}, {-1, -1, -1}, {1, 1, 1}};
        third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    }

    const auto tMesh = mesh::Mesh{kMeshFile};
    const auto tNodalDensitiesAllOne = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 1.0);
    const auto tMeshDesignVariablesAllOne = mesh::DesignVariablesConversion{tMesh}.nodalDensitiesToMeshDesignVariables(
        mesh::NodalDensityVectorReference{tNodalDensitiesAllOne});
    const auto tFilteredControlAllOne =
        tFilterCache.compute(tMeshDesignVariablesAllOne)->filter(tMeshDesignVariablesAllOne);
    const auto tMeshDesignVariablesViewFilteredAllOne = mesh::MeshDesignVariablesDensitiesView{tFilteredControlAllOne};

    // change control and ensure filter size is the same but values are different
    const auto tNodalDensitiesAllHalf = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 0.5);
    const auto tMeshDesignVariablesAllHalf = mesh::DesignVariablesConversion{tMesh}.nodalDensitiesToMeshDesignVariables(
        mesh::NodalDensityVectorReference{tNodalDensitiesAllHalf});
    const auto tFilteredControlAllHalf =
        tFilterCache.compute(tMeshDesignVariablesAllHalf)->filter(tMeshDesignVariablesAllHalf);
    const auto tMeshDesignVariablesViewFilteredAllHalf =
        mesh::MeshDesignVariablesDensitiesView{tFilteredControlAllHalf};

    EXPECT_EQ(tMeshDesignVariablesViewFilteredAllOne.size(), tMeshDesignVariablesViewFilteredAllHalf.size());
    const auto [tFilteredDensitiesAllOne, tIDsAllOne] =
        mesh::detail::split_densities(mesh::mesh_design_variables_to_vector(tMeshDesignVariablesViewFilteredAllOne));
    const auto [tFilteredDensitiesAllHalf, tIDsAllHalf] =
        mesh::detail::split_densities(mesh::mesh_design_variables_to_vector(tMeshDesignVariablesViewFilteredAllHalf));
    EXPECT_NE(tFilteredDensitiesAllOne, tFilteredDensitiesAllHalf);
    EXPECT_EQ(tIDsAllOne, tIDsAllHalf);

    // change mesh and ensure filter size is different
    {
        const auto tCommandGenerator =
            third_party_integration::stk_io::CommandGenerator{{3, 2, 3}, {-1, -1, -1}, {1, 1, 1}};
        third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    }

    const auto tUpdatedMesh = mesh::Mesh{kMeshFile};
    const auto tUpdatedNodalDensitiesAllOne =
        std::vector<double>(mesh::EntityCounts{tUpdatedMesh}.numberOfNodes(), 1.0);
    const auto tUpdatedMeshDesignVariablesAllOne =
        mesh::DesignVariablesConversion{tUpdatedMesh}.nodalDensitiesToMeshDesignVariables(
            mesh::NodalDensityVectorReference{tUpdatedNodalDensitiesAllOne});
    const auto tUpdatedFilteredControlAllOne =
        tFilterCache.compute(tUpdatedMeshDesignVariablesAllOne)->filter(tUpdatedMeshDesignVariablesAllOne);
    const auto tUpdatedMeshDesignVariablesViewFilteredAllOne =
        mesh::MeshDesignVariablesDensitiesView{tUpdatedFilteredControlAllOne};

    EXPECT_NE(tMeshDesignVariablesViewFilteredAllOne.size(), tUpdatedMeshDesignVariablesViewFilteredAllOne.size());

    test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
}

}  // namespace plato::filter::extension::unittest
