#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_TESTUTILITIES_KRINOTESTFIXTURE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_TESTUTILITIES_KRINOTESTFIXTURE

#include <gtest/gtest.h>

#include <utility>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino::test_utilities
{

void test_sensitivity_map(const SensitivityMap& aResult,
                          const SensitivityMap& aGold,
                          const plato::test_utilities::TestContext& aTestContext);

/// @brief A test that can be run in serial and in parallel and produces the same result.
void four_tri_test_on_background_node_ids(const plato::test_utilities::TestContext& aTestContext);

/// @brief Takes a vector of doubles value @a aVector along with a BackgroundMesh @a aBackgroundMesh and sets the level
/// set field for all background nodes (fixed or design) to the values in the vector in order of GID.
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', followed by
/// 'initialize_background_mesh' to setup the level set fields in the krino mesh.
/// @pre Size of `MPI_COMM_WORLD` must be 1.
void populate_level_set_field_from_vector(const BackgroundMesh& aBackgroundMesh, const std::vector<double>& aVector);

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
