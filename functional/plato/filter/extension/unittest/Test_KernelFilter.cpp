#include <gtest/gtest.h>
#include <time.h>

#include <boost/math/constants/constants.hpp>
#include <boost/mpi/communicator.hpp>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/filter/library/HashGeneration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

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
    const auto tAnalysisDomainMesh =
        tMesh.nodalFieldToAnalysisDomainMesh(mesh::NodalFieldVectorReference{tNodalDensities});

    const auto tResult = tKernelFilter.filter(tAnalysisDomainMesh);
    const auto [tPostFilter, tIDMap] = analysis::split_scalar_field_values(
        analysis::mesh_analysis_to_vector(analysis::AnalysisDomainMeshSequentialView{tResult}));

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
            .rowVectorTimesJacobian(tAnalysisDomainMesh, linear_algebra::DynamicVector<double>(tStdVectorSensitivities))
            .stdVector();

    test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));

    return {tPostFilter, tPostSensitivities};
}

double timing_test(const unsigned int aNumberOfElementsOnSide)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {aNumberOfElementsOnSide, aNumberOfElementsOnSide, aNumberOfElementsOnSide},
        {0, 0, 0},
        {(double)aNumberOfElementsOnSide, (double)aNumberOfElementsOnSide, (double)aNumberOfElementsOnSide}};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);
    const FilterRadius tFilterRadius{3.1};

    clock_t t1 = clock();
    [[maybe_unused]] const KernelFilter tKernelFilter{mesh::Mesh{kMeshFile}, tFilterRadius,
                                                      input_parser::KernelFilterCenteringTypes::kElementCentered,
                                                      boost::mpi::communicator{}};
    clock_t t2 = clock();
    return 1000.0 * (t2 - t1) / CLOCKS_PER_SEC;
}

}  // namespace

TEST(KernelFilter, Timing)
{
    std::vector<unsigned int> tNSide = {3, 5, 8, 10, 12, 14, 20, 22};
    for (const auto& x : tNSide)
    {
        std::cout << "N: " << x << std::endl;
        const auto tTime = timing_test(x);
        std::cout << "Total time: " << tTime << std::endl;
    }
}

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

    const auto tFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
    ASSERT_TRUE(tFilePath.has_value());

    const auto tMesh = mesh::Mesh{tFilePath.value()};
    const auto tNodalDensities = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 1.0);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalDensities});

    ASSERT_NO_THROW([[maybe_unused]] const auto tFilter = tFilterCache.compute(tAnalysisDomainMesh));
}

TEST(KernelFilterDetail, FilterCache_DummyCallCounts)
{
    // dummy filter cache
    std::size_t tCallCount{0};
    filter::library::FilterCache tCache{[&tCallCount](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                        {
                                            ++tCallCount;
                                            return std::make_shared<KernelFilter>(
                                                mesh::Mesh{aAnalysisDomainMesh}, FilterRadius{1.0},
                                                input_parser::KernelFilterCenteringTypes::kElementCentered,
                                                boost::mpi::communicator{});
                                        },
                                        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                        { return library::hash_mesh_coordinates(aAnalysisDomainMesh); }};

    // make mesh
    const auto tFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
    ASSERT_TRUE(tFilePath.has_value());

    const auto tMesh = mesh::Mesh{tFilePath.value()};
    const auto tNodalDensitiesAllOne = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 1.0);
    const auto tAnalysisDomainMeshAllOne = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalDensitiesAllOne});

    // apply cache
    EXPECT_EQ(tCallCount, 0);
    tCache.compute(tAnalysisDomainMeshAllOne);
    EXPECT_EQ(tCallCount, 1);
    tCache.compute(tAnalysisDomainMeshAllOne);
    EXPECT_EQ(tCallCount, 1);

    // change density values and apply
    const auto tNodalDensitiesAllHalf = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 0.5);
    const auto tAnalysisDomainMeshAllHalf = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalDensitiesAllHalf});
    tCache.compute(tAnalysisDomainMeshAllHalf);
    EXPECT_EQ(tCallCount, 1);
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
    const auto tAnalysisDomainMeshAllOne = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalDensitiesAllOne});
    const auto tFilteredControlAllOne =
        tFilterCache.compute(tAnalysisDomainMeshAllOne)->filter(tAnalysisDomainMeshAllOne);
    const auto tAnalysisDomainMeshViewFilteredAllOne =
        analysis::AnalysisDomainMeshSequentialView{tFilteredControlAllOne};

    // change control and ensure filter size is the same but values are different
    const auto tNodalDensitiesAllHalf = std::vector<double>(mesh::EntityCounts{tMesh}.numberOfNodes(), 0.5);
    const auto tAnalysisDomainMeshAllHalf = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalDensitiesAllHalf});
    const auto tFilteredControlAllHalf =
        tFilterCache.compute(tAnalysisDomainMeshAllHalf)->filter(tAnalysisDomainMeshAllHalf);
    const auto tAnalysisDomainMeshViewFilteredAllHalf =
        analysis::AnalysisDomainMeshSequentialView{tFilteredControlAllHalf};

    EXPECT_EQ(tAnalysisDomainMeshViewFilteredAllOne.size(), tAnalysisDomainMeshViewFilteredAllHalf.size());
    const auto [tFilteredDensitiesAllOne, tIDsAllOne] =
        analysis::split_scalar_field_values(analysis::mesh_analysis_to_vector(tAnalysisDomainMeshViewFilteredAllOne));
    const auto [tFilteredDensitiesAllHalf, tIDsAllHalf] =
        analysis::split_scalar_field_values(analysis::mesh_analysis_to_vector(tAnalysisDomainMeshViewFilteredAllHalf));
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
    const auto tUpdatedAnalysisDomainMeshAllOne =
        mesh::DesignVariablesConversion{tUpdatedMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{tUpdatedNodalDensitiesAllOne});
    const auto tUpdatedFilteredControlAllOne =
        tFilterCache.compute(tUpdatedAnalysisDomainMeshAllOne)->filter(tUpdatedAnalysisDomainMeshAllOne);
    const auto tUpdatedAnalysisDomainMeshViewFilteredAllOne =
        analysis::AnalysisDomainMeshSequentialView{tUpdatedFilteredControlAllOne};

    EXPECT_NE(tAnalysisDomainMeshViewFilteredAllOne.size(), tUpdatedAnalysisDomainMeshViewFilteredAllOne.size());

    test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
}

}  // namespace plato::filter::extension::unittest
