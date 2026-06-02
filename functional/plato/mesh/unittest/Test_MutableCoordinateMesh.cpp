#include <gtest/gtest.h>

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MutableCoordinateMesh.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh::unittest
{
namespace
{
using MutableCoordinateMeshFixture = third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;
}

TEST_F(MutableCoordinateMeshFixture, UpdateNodalCoordinates)
{
    const auto tOriginalMesh = Mesh{mMeshFilePath};
    const auto tShift = third_party_integration::common::Coordinate{0.01, 0.02, 0.0};
    auto tNewCoordinates = EntityRetrieval{tOriginalMesh}.nodalCoordinates();
    std::transform(tNewCoordinates.begin(), tNewCoordinates.end(), tNewCoordinates.begin(),
                   [&](const auto& aCoordinate) { return aCoordinate + tShift; });

    auto tMutableCoordinateMesh = MutableCoordinateMesh{tOriginalMesh};
    tMutableCoordinateMesh.updateNodalCoordinates(tNewCoordinates);
    const auto tScratchFileName = std::filesystem::path{"scratch.exo"};
    tMutableCoordinateMesh.writeMeshToDisk(tScratchFileName);
    const auto tWrittenCoordinates = EntityRetrieval{Mesh{tScratchFileName}}.nodalCoordinates();

    for (const auto& [tWritten, tShifted] : utilities::Zip(tWrittenCoordinates, tNewCoordinates))
    {
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tWritten, tShifted, TEST_CONTEXT("Coordinates written to file are the same as the shifted coordinates."));
    }
    std::filesystem::remove(tScratchFileName);
}

}  // namespace plato::mesh::unittest
