#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <vector>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/SweptFilter.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/filter/test_utilities/FilterFunction.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{

const auto kDensityInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
const auto kSquareSourceMeshFile = utilities::data_file_path("square_2d.cdf");
const auto kBarTargetMeshFile = std::filesystem::path{"target.exo"};
const auto kNumberOfDesignNodes = 36U;

void create_bar_mesh()
{
    namespace tpis = third_party_integration::stk_io;
    const auto tCommandGenerator =
        tpis::CommandGenerator{{10, 10, 5}, {-5, -5, 0}, {5, 5, 5}, tpis::CommandElementType::Hex};
    tpis::write_mesh(kBarTargetMeshFile, tCommandGenerator.toString());
}

auto make_test_z_swept_filter()
{
    const auto tRadius = 3.25;
    const auto tSweptInput =
        filter::extension::kernel_filters::test_utilities::make_filter_input<input_parser::z_swept_filter>(
            tRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered);
    const auto tSweptFilterType = filter::extension::kernel_filters::make_z_swept_filter_type(
        tSweptInput, analysis::AnalysisDomainMesh{kSquareSourceMeshFile.value(), {}});

    const auto tFilter =
        std::make_shared<filter::extension::kernel_filters::KernelFilter<input_parser::z_swept_filter>>(
            filter::extension::kernel_filters::SourceMesh{mesh::Mesh{kSquareSourceMeshFile.value()}},
            filter::extension::kernel_filters::TargetMesh{mesh::Mesh{kBarTargetMeshFile}}, tSweptFilterType,
            boost::mpi::communicator{});

    return filter::test_utilities::make_filter_function(tFilter);
}

const auto kDensityTopologyInputLambda = []()
{
    auto tInput = kDensityInput;
    tInput.mesh_name = input_parser::FileName{kSquareSourceMeshFile.value()};
    return tInput;
};

}  // namespace

TEST(DensityTopology, InitialGuessWithSymmetryFilter)
{
    const auto tInput = kDensityTopologyInputLambda();

    const auto tInitialGuess = DensityTopology::initialGuess(tInput);
    EXPECT_EQ(tInitialGuess.size(),
              mesh::EntityCounts{mesh::Mesh{kSquareSourceMeshFile.value()}}.numberOfDesignDomainNodes());
}

TEST(DensityTopology, GenerateMeshWithSymmetryFilter)
{
    create_bar_mesh();
    const auto tInput = kDensityTopologyInputLambda();
    const auto tFilter = make_test_z_swept_filter();
    const auto tDensityTopology = DensityTopology{tInput, tFilter};

    auto tUnfiltered = std::vector<double>(kNumberOfDesignNodes, 0.0);
    tUnfiltered[26] = 1;
    tUnfiltered[27] = 1;
    tUnfiltered[25] = 1;
    const auto tThreeDAnalysisDomainMesh =
        tDensityTopology.generateMesh(linear_algebra::DynamicVector<double>{tUnfiltered});

    EXPECT_EQ(tThreeDAnalysisDomainMesh.mFileName, kBarTargetMeshFile);
    const auto tTargetMesh = mesh::Mesh{kBarTargetMeshFile};
    const auto tDensities =
        analysis::mesh_analysis_to_vector(analysis::AnalysisDomainMeshSequentialView{tThreeDAnalysisDomainMesh});

    EXPECT_EQ(tDensities.size(), mesh::EntityCounts{mesh::Mesh{tTargetMesh}}.numberOfDesignDomainNodes());
    std::filesystem::remove(kBarTargetMeshFile);
}

namespace
{
const auto kNumberOfZeroZCoordinates = 121U;
[[nodiscard]] auto zero_z_index(unsigned int aCoordinateIndex) -> unsigned int
{
    return aCoordinateIndex % kNumberOfZeroZCoordinates;
}
}  // namespace

TEST(DensityTopology, OutputWithSymmetryFilter)
{
    create_bar_mesh();
    auto tSolutionVector = std::vector<double>(kNumberOfDesignNodes, 0.0);
    std::iota(tSolutionVector.begin(), tSolutionVector.end(), 1.0);

    const auto tInput = kDensityTopologyInputLambda();
    const auto tFilter = make_test_z_swept_filter();
    const auto tOutputInfo = output::kOverwriteInfo;

    DensityTopology::output(linear_algebra::DynamicVector<double>{tSolutionVector}, tFilter, tInput, tOutputInfo);
    ASSERT_TRUE(std::filesystem::exists(kDensityInput.output_name.value().mToken));
    const auto tFilteredField =
        mesh::EntityRetrieval{mesh::Mesh{kDensityInput.output_name.value().mToken}}.designDomainNodalField(
            filtered_density_mesh_field_name());

    const auto tCoordinatesFromBar =
        mesh::EntityRetrieval{mesh::Mesh{kDensityInput.output_name.value().mToken}}.designDomainNodalCoordinates();

    for (const auto& [tIndex, tValue, tCoordinate] : utilities::enumerate(tFilteredField, tCoordinatesFromBar))
    {
        EXPECT_DOUBLE_EQ(tValue, tFilteredField[zero_z_index(tIndex)])
            << tCoordinate << " and base " << tCoordinatesFromBar[zero_z_index(tIndex)];
    }

    std::filesystem::remove(kBarTargetMeshFile);
    std::filesystem::remove(kDensityInput.output_name.value().mToken);
    std::filesystem::remove(restart_file_name(kDensityInput));
}

}  // namespace plato::geometry::extension::unittest
