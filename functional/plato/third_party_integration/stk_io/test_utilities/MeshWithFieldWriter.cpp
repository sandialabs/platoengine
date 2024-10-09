#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

#include <numeric>

#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
namespace
{
const CommandGenerator kCommandGenerator{{3, 1, 1}, {0, 0, 0}, {1, 1, 1}, CommandElementType::Hex};
const std::string_view kSourceMeshName = "source.exo";
const std::string_view kFieldName = "Topology";
const std::string_view kNodalMeshName = "nodal_density.exo";
const std::string_view kElementMeshName = "element_density.exo";

[[nodiscard]] auto create_nodal_gold() -> std::map<std::size_t, double>
{
    std::vector<std::size_t> tGoldVector(kCommandGenerator.numberOfNodes());
    std::iota(tGoldVector.begin(), tGoldVector.end(), 1U);
    std::map<std::size_t, double> tGold;
    std::transform(tGoldVector.begin(), tGoldVector.end(), std::inserter(tGold, tGold.end()),
                   [](const auto& aIndex) { return std::make_pair(aIndex, static_cast<double>(aIndex)); });

    return tGold;
}

}  // namespace

MeshWithDensities::MeshWithDensities(const std::map<std::size_t, double>& aGoldNumbering)
    : mFieldName(kFieldName), mGoldNumbering(aGoldNumbering)
{
}

MeshWithDensities::~MeshWithDensities() { std::filesystem::remove(kSourceMeshName); }

MeshWithElementDensities::MeshWithElementDensities()
    : MeshWithDensities({{1U, 1.0}, {2U, 2.0}, {3U, 3.0}}), mMeshName(std::string{kElementMeshName})
{
    detail::create_element_density_field_mesh_for_reading(kCommandGenerator, std::string{kElementMeshName});
}
MeshWithElementDensities::~MeshWithElementDensities() { std::filesystem::remove(kElementMeshName); }

MeshWithNodalDensities::MeshWithNodalDensities()
    : MeshWithDensities(create_nodal_gold()), mMeshName(std::string{kNodalMeshName})
{
    detail::create_node_density_field_mesh_for_reading(kCommandGenerator, std::string{kNodalMeshName});
}
MeshWithNodalDensities::~MeshWithNodalDensities() { std::filesystem::remove(kNodalMeshName); }

namespace detail
{
void create_element_density_field_mesh_for_reading(const CommandGenerator& aCommandGenerator,
                                                   const std::filesystem::path& aFileName)
{
    write_mesh(std::string{kSourceMeshName}, aCommandGenerator.toString());
    std::vector<double> tElementDensities(aCommandGenerator.numberOfElements());
    std::iota(tElementDensities.begin(), tElementDensities.end(), 1.0);
    write_element_scalar_field(
        std::string{kSourceMeshName}, [&tElementDensities](const auto aIndex) { return tElementDensities[aIndex - 1]; },
        kFieldName, aFileName);
}

void create_node_density_field_mesh_for_reading(const CommandGenerator& aCommandGenerator,
                                                const std::filesystem::path& aFileName)
{
    write_mesh(std::string{kSourceMeshName}, aCommandGenerator.toString());
    std::vector<double> tDensities(aCommandGenerator.numberOfNodes());
    std::iota(tDensities.begin(), tDensities.end(), 1.0);
    write_nodal_scalar_field(
        std::string{kSourceMeshName}, [&tDensities](const auto aIndex) { return tDensities[aIndex - 1]; }, kFieldName,
        aFileName);
}
}  // namespace detail
}  // namespace plato::third_party_integration::stk_io::test_utilities
