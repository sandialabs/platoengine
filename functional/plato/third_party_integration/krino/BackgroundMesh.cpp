#include "plato/third_party_integration/krino/BackgroundMesh.hpp"

#include <Akri_CDFEM_Support.hpp>
#include <Akri_FieldRef.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_LevelSetShapeSensitivities.hpp>
#include <Akri_MeshHelpers.hpp>
#include <Akri_MeshInterface.hpp>
#include <Akri_NodalSurfaceDistance.hpp>
#include <Akri_Phase_Support.hpp>
#include <cassert>
#include <filesystem>
#include <memory>
#include <set>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <string>
#include <utility>
#include <vector>

#include "plato/third_party_integration/krino/BlockUtilities.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/MeshFromFileWithoutFields.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kDecompositionMethod = std::string_view{"rib"};
constexpr auto kNumberOfLevelSets = 1U;

void setup_fields_for_conforming_decomposition(stk::mesh::MetaData& aMeta,
                                               const std::vector<::krino::LS_Field>& aLevelSetFields)
{
    ::krino::CDFEM_Support& tCdfemSupport = ::krino::CDFEM_Support::get(aMeta);
    const ::krino::FieldRef tCoordsField = aMeta.coordinate_field();

    tCdfemSupport.register_cdfem_snap_displacements_field();
    ::krino::create_levelset_copies_and_set_to_use_as_snap_fields(aMeta, aLevelSetFields);

    tCdfemSupport.set_coords_field(tCoordsField);
    tCdfemSupport.add_edge_interpolation_field(tCoordsField);
    tCdfemSupport.register_parent_node_ids_field();
    tCdfemSupport.finalize_fields();
}

auto krino_mesh_from_file(const std::filesystem::path& aFilename) -> std::unique_ptr<::krino::MeshInterface>
{
    assert(std::filesystem::exists(aFilename));
    return std::make_unique<MeshFromFileWithoutFields>(aFilename.string(), stk::EnvData::parallel_comm(),
                                                       std::string{kDecompositionMethod});
}

auto activate_krino_fields(const std::unique_ptr<::krino::MeshInterface>& aMeshFromFile) -> void
{
    aMeshFromFile->populate_mesh();
    ::krino::activate_all_entities(aMeshFromFile->bulk_data(),
                                   ::krino::AuxMetaData::get(aMeshFromFile->meta_data()).active_part());
}

auto initialize_background_mesh(const std::filesystem::path& aFileName,
                                const std::set<std::string>& aExcludedBlocks) -> BackgroundMesh
{
    auto tMeshFromFile = krino_mesh_from_file(aFileName);

    auto tLevelSetFields =
        ::krino::LSPerInterfacePolicy::setup_levelsets_on_blocks_with_void_phase_for_any_negative_levelset(
            tMeshFromFile->meta_data(), kNumberOfLevelSets, all_blocks_except(*tMeshFromFile, aExcludedBlocks));

    setup_fields_for_conforming_decomposition(tMeshFromFile->meta_data(), tLevelSetFields);
    activate_krino_fields(tMeshFromFile);

    return BackgroundMesh{std::move(tMeshFromFile), std::move(tLevelSetFields)};
}
}  // namespace

auto background_mesh_with_zero_level_set_field(const std::filesystem::path& aFileName,
                                               const std::set<std::string>& aExcludedBlocks) -> BackgroundMesh
{
    return initialize_background_mesh(aFileName, aExcludedBlocks);
}

auto background_mesh_from_primitives(const std::filesystem::path& aFileName,
                                     const LevelSetPrimitives& aLevelSetPrimitives,
                                     const std::set<std::string>& aExcludedBlocks) -> BackgroundMesh
{
    auto tBackgroundMesh = initialize_background_mesh(aFileName, aExcludedBlocks);

    auto tInitializationSurfaces = make_krino_composite_surface();
    append_spheres(tInitializationSurfaces, aLevelSetPrimitives.mSpheres);
    append_planes(tInitializationSurfaces, aLevelSetPrimitives.mPlanes);
    ::krino::compute_nodal_surface_distance(tBackgroundMesh.mKrinoMesh->bulk_data(),
                                            tBackgroundMesh.mKrinoMesh->bulk_data().mesh_meta_data().coordinate_field(),
                                            tBackgroundMesh.mLevelSetFields.front().isovar, tInitializationSurfaces);
    return tBackgroundMesh;
}
}  // namespace plato::third_party_integration::krino
