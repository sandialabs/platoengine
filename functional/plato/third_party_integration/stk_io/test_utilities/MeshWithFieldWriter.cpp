#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

#include <numeric>

#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
namespace
{
constexpr CommandGenerator kCommandGenerator{{3, 1, 1}, {0, 0, 0}, {1, 1, 1}, CommandElementType::Hex};
constexpr std::string_view kTestDirectoryName = "test";
const auto kSourceMeshPath = std::filesystem::path{kTestDirectoryName} / std::filesystem::path{"source.exo"};
constexpr std::string_view kFieldName = "Topology";
constexpr std::string_view kNodalMeshName = "nodal_density.exo";
constexpr std::string_view kElementMeshName = "element_density.exo";

[[nodiscard]] auto create_nodal_gold() -> std::map<std::size_t, double>
{
    std::vector<std::size_t> tGoldVector(kCommandGenerator.numberOfNodes());
    std::iota(tGoldVector.begin(), tGoldVector.end(), 1U);
    std::map<std::size_t, double> tGold;
    std::transform(tGoldVector.begin(), tGoldVector.end(), std::inserter(tGold, tGold.end()),
                   [](const auto& aIndex) { return std::make_pair(aIndex, static_cast<double>(aIndex)); });

    return tGold;
}

void create_element_density_field_mesh_for_reading(const CommandGenerator& aCommandGenerator,
                                                   const std::filesystem::path& aFileName,
                                                   const std::string_view aFieldName)
{
    write_mesh(kSourceMeshPath, aCommandGenerator);
    std::vector<double> tElementDensities(aCommandGenerator.numberOfElements());
    std::iota(tElementDensities.begin(), tElementDensities.end(), 1.0);
    test_utilities::write_element_scalar_field(
        kSourceMeshPath, [&tElementDensities](const auto aIndex) { return tElementDensities[aIndex - 1]; }, aFieldName,
        aFileName);
}

void create_node_density_field_mesh_for_reading(const CommandGenerator& aCommandGenerator,
                                                const std::filesystem::path& aFileName,
                                                const std::string_view aFieldName)
{
    write_mesh(kSourceMeshPath, aCommandGenerator);
    std::vector<double> tDensities(aCommandGenerator.numberOfNodes());
    std::iota(tDensities.begin(), tDensities.end(), 1.0);
    write_nodal_scalar_field(
        kSourceMeshPath, [&tDensities](const auto aIndex) { return tDensities[aIndex - 1]; }, aFieldName, aFileName);
}
}  // namespace

MeshWithDensities::MeshWithDensities(const std::map<std::size_t, double>& aGoldNumbering,
                                     const std::filesystem::path& aMeshName,
                                     const std::string_view aFieldName)
    : mFieldName{aFieldName},
      mGoldNumbering{aGoldNumbering},
      mDirectory{kTestDirectoryName},
      mMeshName{mDirectory.directory() / aMeshName}
{
}

MeshWithElementDensities::MeshWithElementDensities()
    : MeshWithDensities({{1U, 1.0}, {2U, 2.0}, {3U, 3.0}}, kElementMeshName, kFieldName)
{
    mDirectory.writeFile([this](const std::filesystem::path& aPath)
                         { create_element_density_field_mesh_for_reading(kCommandGenerator, aPath, mFieldName); },
                         mMeshName.filename());
}

MeshWithNodalDensities::MeshWithNodalDensities() : MeshWithNodalDensities(kNodalMeshName, kFieldName) {}

MeshWithNodalDensities::MeshWithNodalDensities(const std::filesystem::path& aMeshName,
                                               const std::string_view aFieldName)
    : MeshWithDensities(create_nodal_gold(), aMeshName, aFieldName)
{
    mDirectory.writeFile([this](const std::filesystem::path& aPath)
                         { create_node_density_field_mesh_for_reading(kCommandGenerator, aPath, mFieldName); },
                         mMeshName.filename());
}
}  // namespace plato::third_party_integration::stk_io::test_utilities
