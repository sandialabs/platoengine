#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/HashGeneration.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
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

[[nodiscard]] auto test_filter_evaluation(const input_parser::KernelFilterCenteringTypes aFilterCentering)
    -> std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
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

    const auto tNodalSensitivities = [&tCommandGenerator]()
    {
        auto tSensitivities = std::vector<double>(tCommandGenerator.numberOfNodes(), 0);
        tSensitivities[0] = 1;
        tSensitivities[1] = .5;
        tSensitivities[2] = .25;
        return linear_algebra::DynamicVector<double>(std::move(tSensitivities));
    }();
    const auto tElementSensitivities = linear_algebra::DynamicVector<double>(tCommandGenerator.numberOfElements(), 1);

    const auto tPostSensitivities =
        aFilterCentering == input_parser::KernelFilterCenteringTypes::kElementCentered
            ? tKernelFilter.rowVectorTimesJacobian(tAnalysisDomainMesh, tElementSensitivities).stdVector()
            : tKernelFilter.rowVectorTimesJacobian(tAnalysisDomainMesh, tNodalSensitivities).stdVector();
    const auto tPostAdjointSensitivities =
        tKernelFilter.rowVectorTimesAdjointJacobian(tAnalysisDomainMesh, tNodalSensitivities).stdVector();

    plato::test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));

    return {tPostFilter, tPostSensitivities, tPostAdjointSensitivities};
}

}  // namespace

TEST(KernelFilter, SingleHexElementCentered)
{
    /*
        distance_matrix = [1/8 1/8 1/8 1/8 1/8 1/8 1/8 1/8]
        % Filter application
        rho = [0 0 0 .5 1 .5 0 0]'
        distance_matrix * rho
          % = 1/4

        % Jacobian
        v = [1]
        v*distance_matrix
         % = [1/8 1/8 1/8 1/8 1/8 1/8 1/8 1/8]

        % Adjoint Jacobian
        u = [1 .5 .25 0 0 0 0 0 ]
        u * distance_matrix'
         % = 0.21875
    */

    const auto [tResultFilter, tResultJV, tResultAdjointJV] =
        test_filter_evaluation(input_parser::KernelFilterCenteringTypes::kElementCentered);

    ASSERT_EQ(tResultFilter.size(), 1u);
    EXPECT_NEAR(tResultFilter[0], 1.0 / 4.0, kTolerance);

    ASSERT_EQ(tResultJV.size(), 8u);
    const auto tExpectedVectorJacobianProduct = std::vector<double>(8U, 1.0 / 8.0);
    plato::test_utilities::expect_container_entries_near(tExpectedVectorJacobianProduct, tResultJV, kTolerance,
                                                         TEST_CONTEXT("Vector Jacobian product"));

    ASSERT_EQ(tResultAdjointJV.size(), 1U);
    EXPECT_NEAR(tResultAdjointJV.front(), 0.21875, kTolerance);
}

TEST(KernelFilter, SingleHexNodalCentered)
{
    // clang-format off
/*
    distance_matrix = [1        .1/1.1   .1/1.1      0           .1/1.1      0           0            0
                       .1/1.1   1        0           .1/1.1      0           .1/1.1      0            0
                       .1/1.1   0        1           .1/1.1      0           0          .1/1.1        0
                       0        .1/1.1   .1/1.1      1           0           0           0            .1/1.1
                       .1/1.1   0        0           0           1           .1/1.1      .1/1.1       0
                       0        .1/1.1   0           0           .1/1.1      1           0            .1/1.1
                       0        0        .1/1.1      0           .1/1.1      0           1            .1/1.1
                       0        0        0           .1/1.1      0           .1/1.1      .1/1.1       1 ];
    normalization = sum(distance_matrix, 2);
    filter = distance_matrix ./ normalization;
    % Filter application
    rho = [0 0 0 .5 1 .5 0 0]'
    filter*rho
      % = [ 0.0714285714285714
      %     0.0714285714285714
      %     0.0357142857142857
      %     0.392857142857143
      %     0.821428571428571
      %     0.464285714285714
      %     0.0714285714285714
      %     0.0714285714285714 ] 
    
    % Jacobian
    v = [1 .5 .25 0 0 0 0 0 ]
    v*filter 
      % = [0.839285714285714 0.464285714285714 0.267857142857143 0.0535714285714286 0.0714285714285714 0.0357142857142857 0.0178571428571429 0]

    % Adjoint Jacobian
    u = [1 2 3 4 5 6 7 8]
    u * filter'
     % = [1.5 2.357142857142858 3.214285714285715 4.071428571428571 4.928571428571429 5.785714285714286 6.642857142857142 7.5]
*/
    // clang-format on

    const auto [tResultFilter, tResultVectorJacobianProduct, tResultVectorAdjointJacobianProduct] =
        test_filter_evaluation(input_parser::KernelFilterCenteringTypes::kNodeCentered);

    const auto tExpectedFilter =
        std::vector{7.142857142857144e-02, 7.142857142857144e-02, 3.571428571428572e-02, 3.928571428571428e-01,
                    8.214285714285715e-01, 4.642857142857143e-01, 7.142857142857144e-02, 7.142857142857144e-02};
    plato::test_utilities::expect_container_entries_near(tResultFilter, tExpectedFilter, kTolerance,
                                                         TEST_CONTEXT("Filter application"));

    const auto tExpectedVectorJacobianProduct =
        std::vector{8.392857142857144e-01, 4.642857142857144e-01, 2.678571428571429e-01, 5.357142857142858e-02,
                    7.142857142857144e-02, 3.571428571428572e-02, 1.785714285714286e-02, 0.0};
    plato::test_utilities::expect_container_entries_near(tResultVectorJacobianProduct, tExpectedVectorJacobianProduct,
                                                         kTolerance, TEST_CONTEXT("Vector Jacobian product"));

    plato::test_utilities::expect_container_entries_near(tResultVectorAdjointJacobianProduct,
                                                         tExpectedVectorJacobianProduct, kTolerance,
                                                         TEST_CONTEXT("Vector adjoint Jacobian product"));
}

TEST(KernelFilter, ProperlyAllocatesMemoryFor2DMesh)
{
    auto tInput = test_utilities::create_valid_kernel_filter_input();
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
                                        { return mesh::hash_mesh_coordinates(aAnalysisDomainMesh); }};

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
    auto tFilterCache = detail::create_filter_cache(test_utilities::create_valid_kernel_filter_input());

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

    plato::test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
}

TEST(KernelFilter, Registration) { EXPECT_TRUE(library::is_filter_function_registered("kernel_filter")); }

}  // namespace plato::filter::extension::unittest
