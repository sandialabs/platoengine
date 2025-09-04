#include "plato/third_party_integration/krino/LevelSetInitialization.hpp"

#include <Akri_AuxMetaData.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_MeshInterface.hpp>
#include <Akri_Phase_Support.hpp>
#include <algorithm>

#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
// Note that a lot of this code is lifted from krino/krino/krino_lib/Akri_LevelSetPolicy.cpp, but modified so that we
// can set the level-set field names for a sub-set of the blocks in the mesh.

constexpr auto kNumberOfStates = 1U;

void declare_and_append_levelset_field(::krino::AuxMetaData& aAuxMeta,
                                       const std::string& aLevelSetName,
                                       std::vector<::krino::LS_Field>& aLevelSetFields)
{
    const unsigned tLevelSetIndex = aLevelSetFields.size();
    auto tLevelSetField =
        aAuxMeta.declare_field(aLevelSetName, ::krino::FieldType::REAL, stk::topology::NODE_RANK, kNumberOfStates);
    constexpr auto tLevelSetIsoValue = 0.0;
    aLevelSetFields.emplace_back(aLevelSetName, ::krino::Surface_Identifier(tLevelSetIndex), tLevelSetField,
                                 tLevelSetIsoValue);
}

[[nodiscard]] auto level_set_field_name(const unsigned aFieldIndex, const unsigned aNumberOfLevelSets) -> std::string
{
    auto tPostFix = aNumberOfLevelSets > 1 ? std::to_string(aFieldIndex) : std::string{};
    return std::string{"LS" + std::move(tPostFix)};
}

[[nodiscard]] auto declare_levelset_fields_and_add_as_interpolation_fields(
    stk::mesh::MetaData& aMetaData, const unsigned aNumberOfLevelSets) -> std::vector<::krino::LS_Field>
{
    auto& tAuxMeta = ::krino::AuxMetaData::get(aMetaData);
    auto& tCdfemSupport = ::krino::CDFEM_Support::get(aMetaData);

    auto tLevelSetFields = std::vector<::krino::LS_Field>{};
    for (const auto tLevelSetIndex : utilities::IndexRange{aNumberOfLevelSets})
    {
        declare_and_append_levelset_field(tAuxMeta, level_set_field_name(tLevelSetIndex, aNumberOfLevelSets),
                                          tLevelSetFields);
    }

    for (const auto& tLevelSetField : tLevelSetFields)
    {
        tCdfemSupport.add_interpolation_field(tLevelSetField.isovar);
    }

    return tLevelSetFields;
}

[[nodiscard]] auto create_named_phases_with_void_phase_for_any_negative_levelset(const unsigned aNumberOfLevelSets)
    -> ::krino::PhaseVec
{
    auto tNamedPhases = ::krino::PhaseVec{};
    const unsigned tNumberOfPhases = 1 << aNumberOfLevelSets;
    for (const auto tPhaseIndex : utilities::IndexRange{tNumberOfPhases})
    {
        auto tPhaseName = std::string{};
        auto tTag = ::krino::PhaseTag{};
        for (const auto tLevelSetIndex : utilities::IndexRange{aNumberOfLevelSets})
        {
            const auto tLevelSetIsNegative = (tPhaseIndex >> tLevelSetIndex) % 2 == 0;
            const auto tLevelSetSign = tLevelSetIsNegative ? -1 : 1;
            tTag.add(::krino::Surface_Identifier(tLevelSetIndex), tLevelSetSign);
            tPhaseName = (tLevelSetIsNegative ? "void" : "");
        }
        tNamedPhases.push_back(::krino::NamedPhase{tPhaseName, tTag});
    }
    return tNamedPhases;
}

void register_blocks_for_decomposition_by_levelsets(::krino::Phase_Support& aPhaseSupport,
                                                    const unsigned aNumberOfLevelSets,
                                                    const stk::mesh::PartVector& aBlocks,
                                                    const ::krino::PhaseVec& aNamedPhases)
{
    for (const auto tLevelSetIndex : utilities::IndexRange{aNumberOfLevelSets})
    {
        aPhaseSupport.register_blocks_for_level_set(::krino::Surface_Identifier(tLevelSetIndex), aBlocks);
    }

    auto tLevelSets = std::vector<
        std::tuple<stk::mesh::PartVector, std::shared_ptr<::krino::Interface_Name_Generator>, ::krino::PhaseVec>>{};
    tLevelSets.push_back(std::make_tuple(aBlocks, std::make_shared<::krino::LS_Name_Generator>(), aNamedPhases));
    aPhaseSupport.decompose_blocks(tLevelSets);
}

void register_levelset_fields(stk::mesh::MetaData& aMetaData,
                              const ::krino::Phase_Support& aPhaseSupport,
                              const std::vector<::krino::LS_Field>& aLevelSetFields)
{
    auto& tAuxMeta = ::krino::AuxMetaData::get(aMetaData);
    for (auto& tField : aLevelSetFields)
    {
        const auto tBlockOrdinals = aPhaseSupport.get_levelset_decomposed_block_ordinals(tField.identifier);
        for (const auto tBlockOrdinal : tBlockOrdinals)
        {
            constexpr auto tNumberOfFieldDimensions = 1U;
            tAuxMeta.register_field(tField.isovar.name(), ::krino::FieldType::REAL, stk::topology::NODE_RANK,
                                    kNumberOfStates, tNumberOfFieldDimensions, aMetaData.get_part(tBlockOrdinal));
        }
    }
}

void setup_phase_support_and_register_levelset_fields(stk::mesh::MetaData& aMetaData,
                                                      const std::vector<::krino::LS_Field>& aLevelSetFields,
                                                      const ::krino::PhaseVec& aNamedPhases,
                                                      const stk::mesh::PartVector& aBlocks,
                                                      ::krino::Block_Surface_Connectivity& aBlockSurfaceInfo)
{
    auto& tPhaseSupport = ::krino::Phase_Support::get(aMetaData);
    constexpr auto tOneLevelSetPerPhase = false;
    tPhaseSupport.set_one_levelset_per_phase(tOneLevelSetPerPhase);
    tPhaseSupport.set_input_block_surface_connectivity(aBlockSurfaceInfo);

    register_blocks_for_decomposition_by_levelsets(tPhaseSupport, aLevelSetFields.size(), aBlocks, aNamedPhases);
    register_levelset_fields(aMetaData, tPhaseSupport, aLevelSetFields);
}
}  // namespace

void setup_level_sets(::krino::MeshInterface& aKrinoMesh, const std::set<std::string>& aExcludedBlocks)
{
    constexpr auto tNumberOfLevelSets = 1U;

    auto& tMetaData = aKrinoMesh.meta_data();
    auto tLevelSetFields = declare_levelset_fields_and_add_as_interpolation_fields(tMetaData, tNumberOfLevelSets);
    const auto tNamedPhases = create_named_phases_with_void_phase_for_any_negative_levelset(tNumberOfLevelSets);

    auto tBlockSurfaceInfo = ::krino::Block_Surface_Connectivity{tMetaData};
    setup_phase_support_and_register_levelset_fields(tMetaData, tLevelSetFields, tNamedPhases,
                                                     all_blocks_except(aKrinoMesh, aExcludedBlocks), tBlockSurfaceInfo);
}

auto all_block_parts(const ::krino::MeshInterface& aKrinoMesh) -> stk::mesh::PartVector
{
    auto tBlockParts = stk::mesh::PartVector{};
    std::ranges::copy_if(aKrinoMesh.meta_data().get_parts(), std::back_inserter(tBlockParts),
                         [](const auto* aPart)
                         {
                             return aPart->primary_entity_rank() == stk::topology::ELEMENT_RANK &&
                                    aPart->subsets().empty() && aPart->topology() != stk::topology::INVALID_TOPOLOGY &&
                                    !stk::mesh::is_auto_declared_part(*aPart);
                         });
    return tBlockParts;
}

[[nodiscard]] auto all_blocks_except(const ::krino::MeshInterface& aKrinoMesh,
                                     const std::set<std::string>& aExcludedBlocks) -> stk::mesh::PartVector
{
    auto tElementBlockParts = all_block_parts(aKrinoMesh);
    std::erase_if(tElementBlockParts,
                  [&aExcludedBlocks](const auto& aPart) { return aExcludedBlocks.contains(aPart->name()); });
    return tElementBlockParts;
}

}  // namespace plato::third_party_integration::krino
