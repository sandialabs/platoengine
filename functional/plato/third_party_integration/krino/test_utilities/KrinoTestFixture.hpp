#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_TESTUTILITIES_KRINOTESTFIXTURE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_TESTUTILITIES_KRINOTESTFIXTURE

#include <gtest/gtest.h>

#include <utility>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino::test_utilities
{

void test_sensitivity_map(const SensitivityMap& aResult,
                          const SensitivityMap& aGold,
                          const plato::test_utilities::TestContext& aTestContext);

/// @brief A test that can be run in serial and in parallel and produces the same result.
void four_tri_test_on_background_node_ids(const plato::test_utilities::TestContext& aTestContext);

/// @brief sets up the appropriate level set fields and activates the krino mesh from a mesh on disk at @a aFileName.
/// Returns a BackgroundMesh containing the mesh meta data and level-set fields with values set from values in
/// @a aVector.
///
/// Excludes blocks with names given in @a aExcludedBlocks. Typically, @a aExcludedBlocks will be the fixed (non-design)
/// blocks.
///
/// @pre The environment for krino was initialized.
/// @pre @a aFileName is a valid exodus mesh file on disk.
/// @pre Size of `MPI_COMM_WORLD` must be 1.
[[nodiscard]] auto background_mesh_from_vector(const std::filesystem::path& aFileName,
                                               const std::vector<double>& aVector,
                                               const std::set<std::string>& aExcludedBlocks = {}) -> BackgroundMesh;

class KrinoTestFixture : virtual public ::testing::Test
{
   protected:
    void SetUp() override
    {
        set_stk_communicator_for_krino(boost::mpi::communicator{MPI_COMM_WORLD, boost::mpi::comm_attach});
    }
};

class SensitivityTestKrinoFixture : public KrinoTestFixture
{
   public:
    SensitivityTestKrinoFixture(std::string aFileName, SensitivityMap aSensitivityMap)
        : mFileName(std::move(aFileName)), mGoldSensitivityMap(std::move(aSensitivityMap))
    {
    }
    void compareMapAgainstBuiltInGold(const SensitivityMap& aResult,
                                      const plato::test_utilities::TestContext& aTestContext) const
    {
        test_sensitivity_map(aResult, mGoldSensitivityMap, aTestContext);
    }

    std::string mFileName;
    SensitivityMap mGoldSensitivityMap;
};

}  // namespace plato::third_party_integration::krino::test_utilities

#endif
