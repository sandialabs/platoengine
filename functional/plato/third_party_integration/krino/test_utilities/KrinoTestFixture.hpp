#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_TESTUTILITIES_KRINOTESTFIXTURE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_TESTUTILITIES_KRINOTESTFIXTURE

#include <gtest/gtest.h>

#include <filesystem>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino::test_utilities
{

namespace detail
{

[[nodiscard]] auto get_log_file_name() -> std::string_view;

void initialize_krino();

}  // namespace detail

void test_sensitivity_map(const SensitivityMap& aResult,
                          const SensitivityMap& aGold,
                          const plato::test_utilities::TestContext& aTestContext);

/// @brief Takes a vector of doubles value @a aVector along with a KrinoMesh @a aKrinoMesh and sets the level set field
/// for all background nodes (fixed or design) to the values in the vector in order of GID.
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino', followed by
/// 'read_and_setup_for_decomposition' to setup the level set fields in the krino mesh.
[[nodiscard]] auto make_level_set_field_from_vector(::krino::MeshInterface& aKrinoMesh,
                                                    const std::vector<double>& aVector)
    -> std::vector<::krino::LS_Field>;

class KrinoTestFixture : virtual public ::testing::Test
{
   protected:
    void SetUp() override { detail::initialize_krino(); }
    void TearDown() override { std::filesystem::remove(std::filesystem::path{detail::get_log_file_name()}); }
};

class SensitivityTestKrinoFixture : public KrinoTestFixture
{
   public:
    SensitivityTestKrinoFixture(const std::string& aFileName, const SensitivityMap& aSensitivityMap)
        : mFileName(aFileName), mGoldSensitivityMap(aSensitivityMap)
    {
    }
    void compareMapAgainstBuiltInGold(const SensitivityMap& aResult,
                                      const plato::test_utilities::TestContext& aTestContext)
    {
        test_sensitivity_map(aResult, mGoldSensitivityMap, aTestContext);
    }

   protected:
    std::string mFileName;

    SensitivityMap mGoldSensitivityMap;
};

}  // namespace plato::third_party_integration::krino::test_utilities

#endif
