#include <Akri_MeshFromFile.hpp>
#include <filesystem>
#include <memory>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string>

namespace plato::third_party_integration::krino
{
/// @brief An implementation of MeshInterface that loads a mesh from disk, but does not read any fields. It is otherwise
/// identical to `krino::MeshFromFile`.
/// @todo This should eventually be merged into the krino library, and so we can remove it.
class MeshFromFileWithoutFields : public ::krino::MeshInterface
{
   public:
    MeshFromFileWithoutFields(const std::filesystem::path& aFileName,
                              stk::ParallelMachine aCommunicator,
                              const std::string& aDecompositionMethod);

    void populate_mesh(
        const stk::mesh::BulkData::AutomaticAuraOption aAutoAuraOption = stk::mesh::BulkData::AUTO_AURA) override;

    [[nodiscard]] auto meta_data() -> stk::mesh::MetaData& override;
    [[nodiscard]] auto meta_data() const -> const stk::mesh::MetaData& override;
    [[nodiscard]] auto bulk_data() -> stk::mesh::BulkData& override;
    [[nodiscard]] auto bulk_data() const -> const stk::mesh::BulkData& override;

   private:
    stk::ParallelMachine mCommunicator;
    std::unique_ptr<stk::io::StkMeshIoBroker> mIOBroker;
};
}  // namespace plato::third_party_integration::krino
