#include "plato/third_party_integration/krino/MeshFromFileWithoutFields.hpp"

#include <Ioss_IOFactory.h>
#include <Ioss_Utils.h>

#include <Akri_AllReduce.hpp>
#include <Akri_AuxMetaData.hpp>
#include <Akri_DiagWriter.hpp>
#include <Akri_MeshFromFile.hpp>
#include <boost/core/null_deleter.hpp>
#include <memory>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/SkinBoundary.hpp>
#include <string>

namespace plato::third_party_integration::krino
{

MeshFromFileWithoutFields::MeshFromFileWithoutFields(const std::filesystem::path& aFileName,
                                                     stk::ParallelMachine aCommunicator,
                                                     const std::string& aDecompositionMethod)
    : mCommunicator{aCommunicator}, mIOBroker{std::make_unique<stk::io::StkMeshIoBroker>(aCommunicator)}
{
    mIOBroker->property_add(Ioss::Property("MAXIMUM_NAME_LENGTH", 180));

    if (!aDecompositionMethod.empty())
    {
        mIOBroker->property_add(Ioss::Property("DECOMPOSITION_METHOD", Ioss::Utils::uppercase(aDecompositionMethod)));
    }

    mIOBroker->add_mesh_database(aFileName.string(), stk::io::READ_MESH);

    if (auto tIORegion = mIOBroker->get_input_ioss_region();
        tIORegion && 3 == tIORegion->get_property("spatial_dimension").get_int())
    {
        mIOBroker->set_enable_all_face_sides_shell_topo(true);
    }
    mIOBroker->create_input_mesh();
    ::krino::AuxMetaData::create(mIOBroker->meta_data());
}

void MeshFromFileWithoutFields::populate_mesh(const stk::mesh::BulkData::AutomaticAuraOption aAutoAuraOption)
{
    auto& tMetaData = mIOBroker->meta_data();
    auto tSharedMetaWeWontDelete = std::shared_ptr<stk::mesh::MetaData>(&tMetaData, boost::null_deleter());
    auto tSharedBulkThatWeGiveToIoBroker = std::shared_ptr{
        stk::mesh::MeshBuilder(mCommunicator).set_aura_option(aAutoAuraOption).create(tSharedMetaWeWontDelete)};

    mIOBroker->set_bulk_data(tSharedBulkThatWeGiveToIoBroker);
    mIOBroker->populate_bulk_data();

    stk::mesh::create_exposed_block_boundary_sides(mIOBroker->bulk_data(), mIOBroker->meta_data().universal_part(),
                                                   {&::krino::AuxMetaData::get(tMetaData).exposed_boundary_part()});
    stk::mesh::create_interior_block_boundary_sides(mIOBroker->bulk_data(), tMetaData.universal_part(),
                                                    {&::krino::AuxMetaData::get(tMetaData).block_boundary_part()});
}

auto MeshFromFileWithoutFields::meta_data() -> stk::mesh::MetaData& { return mIOBroker->meta_data(); }

auto MeshFromFileWithoutFields::meta_data() const -> const stk::mesh::MetaData& { return mIOBroker->meta_data(); }

auto MeshFromFileWithoutFields::bulk_data() -> stk::mesh::BulkData& { return mIOBroker->bulk_data(); }

auto MeshFromFileWithoutFields::bulk_data() const -> const stk::mesh::BulkData& { return mIOBroker->bulk_data(); }

}  // namespace plato::third_party_integration::krino
