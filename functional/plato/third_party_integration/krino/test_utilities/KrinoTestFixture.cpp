#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"

#include <mpi.h>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino::test_utilities
{

namespace
{
const std::string_view kLogFile = "Krino_Test.txt";

}

void test_sensitivity_map(const SensitivityMap& aResult,
                          const SensitivityMap& aGold,
                          const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aResult.size(), aGold.size()) << aTestContext;
    for (const auto& tEntry : aResult)
    {
        const auto& tGoldEntry = aGold.at(tEntry.first);
        ASSERT_EQ(tGoldEntry.mBackgroundMeshNodeIDs, tEntry.second.mBackgroundMeshNodeIDs);
        for (const auto [tGold, tResult] :
             utilities::Zip(tGoldEntry.mNodalSensitivities, tEntry.second.mNodalSensitivities))
        {
            common::test_utilities::test_double_equality_of_components(tResult, tGold, aTestContext);
        }
        EXPECT_EQ(tGoldEntry.mDesignDomainLocalIndex, tEntry.second.mDesignDomainLocalIndex) << aTestContext;
    }
}

auto make_level_set_field_from_vector(::krino::MeshInterface& aKrinoMesh, const std::vector<double>& aVector)
    -> std::vector<::krino::LS_Field>
{
    std::vector<::krino::LS_Field> tField = ::krino::Phase_Support::get_levelset_fields(aKrinoMesh.meta_data());
    const auto tNodes = node_entities_in_mesh(aKrinoMesh, tField);
    assert(tNodes.size() == aVector.size());
    for (const auto [tNode, tValue] : utilities::Zip(tNodes, aVector))
    {
        level_set_value(tField, tNode) = tValue;
    }
    return tField;
}

namespace detail
{

[[nodiscard]] auto get_log_file_name() -> std::string_view { return kLogFile; }

void initialize_krino()
{
    static bool tFirstTime{true};
    if (tFirstTime)
    {
        initialize_environment_for_krino(get_log_file_name(), MPI_COMM_SELF);
        tFirstTime = false;
    }
}

}  // namespace detail

}  // namespace plato::third_party_integration::krino::test_utilities
