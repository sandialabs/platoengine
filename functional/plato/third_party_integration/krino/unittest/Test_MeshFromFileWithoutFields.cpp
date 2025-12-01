#include <gtest/gtest.h>

#include <ranges>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/environment/EnvData.hpp>

#include "plato/third_party_integration/krino/MeshFromFileWithoutFields.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
const auto kMeshFilePath = utilities::data_file_path("mesh_with_field.cdf");
constexpr auto kDecompositionMethod = std::string_view{"rib"};

class MeshFromFileWithoutReadFixture : public krino::test_utilities::KrinoTestFixture
{
};

template <stk::topology::rank_t Rank>
[[nodiscard]] auto entity_size(const stk::mesh::BulkData& aBulk) -> unsigned int
{
    std::vector<size_t> tEntityCounts;
    stk::mesh::comm_mesh_counts(aBulk, tEntityCounts);
    return tEntityCounts[Rank];
}

}  // namespace

TEST_F(MeshFromFileWithoutReadFixture, PopulateMesh)
{
    ASSERT_TRUE(kMeshFilePath);

    auto tMeshFromFile = ::krino::MeshFromFile{kMeshFilePath.value().string(), stk::EnvData::parallel_comm(),
                                               std::string{kDecompositionMethod}};
    tMeshFromFile.populate_mesh();
    const auto tMeshFromFileFields = tMeshFromFile.meta_data().get_fields(stk::topology::rank_t::NODE_RANK);

    auto tMeshFromFileWithoutFields = MeshFromFileWithoutFields{
        kMeshFilePath.value().string(), stk::EnvData::parallel_comm(), std::string{kDecompositionMethod}};
    tMeshFromFileWithoutFields.populate_mesh();
    const auto tMeshFromFileWithoutFieldsFields =
        tMeshFromFileWithoutFields.meta_data().get_fields(stk::topology::rank_t::NODE_RANK);

    EXPECT_LT(tMeshFromFileWithoutFieldsFields.size(), tMeshFromFileFields.size());

    // Check that the meshes are otherwise the same (at least in terms of entity counts)
    EXPECT_EQ(entity_size<stk::topology::rank_t::NODE_RANK>(tMeshFromFile.bulk_data()),
              entity_size<stk::topology::rank_t::NODE_RANK>(tMeshFromFileWithoutFields.bulk_data()));
    EXPECT_EQ(entity_size<stk::topology::rank_t::ELEM_RANK>(tMeshFromFile.bulk_data()),
              entity_size<stk::topology::rank_t::ELEM_RANK>(tMeshFromFileWithoutFields.bulk_data()));
    EXPECT_EQ(entity_size<stk::topology::rank_t::EDGE_RANK>(tMeshFromFile.bulk_data()),
              entity_size<stk::topology::rank_t::EDGE_RANK>(tMeshFromFileWithoutFields.bulk_data()));
    EXPECT_EQ(entity_size<stk::topology::rank_t::FACE_RANK>(tMeshFromFile.bulk_data()),
              entity_size<stk::topology::rank_t::FACE_RANK>(tMeshFromFileWithoutFields.bulk_data()));
}
}  // namespace plato::third_party_integration::krino::unittest
