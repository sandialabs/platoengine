#include "plato/mesh/EntityRetrieval.hpp"

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::mesh
{

namespace
{
[[nodiscard]] auto down_select_values_to_subset(const std::map<std::size_t, double>& aFullField,
                                                const std::vector<std::size_t>& aSubset) -> std::vector<double>
{
    std::vector<double> tSubsetField;
    tSubsetField.reserve(aSubset.size());
    std::transform(aSubset.begin(), aSubset.end(), std::back_inserter(tSubsetField),
                   [&aFullField](const auto& aNodeIndex) { return aFullField.at(aNodeIndex); });
    return tSubsetField;
}
}  // namespace

EntityRetrieval::EntityRetrieval(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto EntityRetrieval::nodalCoordinates() const -> std::vector<third_party_integration::common::Coordinate>
{
    return third_party_integration::stk_io::nodal_coordinates(bulkData());
}

auto EntityRetrieval::elementCentroids() const -> std::vector<third_party_integration::common::Coordinate>
{
    return third_party_integration::stk_io::element_centroids(bulkData());
}

auto EntityRetrieval::designDomainNodalCoordinates() const -> std::vector<third_party_integration::common::Coordinate>
{
    return third_party_integration::stk_io::nodal_coordinates(bulkData(), designDomainBlocks());
}

auto EntityRetrieval::designDomainElementCentroids() const -> std::vector<third_party_integration::common::Coordinate>
{
    return third_party_integration::stk_io::element_centroids(bulkData(), designDomainBlocks());
}

auto EntityRetrieval::designDomainNodeIDs() const -> std::vector<std::size_t>
{
    return third_party_integration::stk_io::node_ids(bulkData(), designDomainBlocks());
}

auto EntityRetrieval::allNodeIDs() const -> std::vector<std::size_t>
{
    auto tAllBlocks = designDomainBlocks();
    const auto tFixedBlocks = fixedDomainBlocks();
    tAllBlocks.insert(tAllBlocks.end(), tFixedBlocks.begin(), tFixedBlocks.end());
    return third_party_integration::stk_io::node_ids(bulkData(), tAllBlocks);
}

auto EntityRetrieval::designDomainNodalField(const std::string_view aFieldName,
                                             const TimeStep aTimeStep) const -> std::vector<double>
{
    namespace tsi = third_party_integration::stk_io;

    const std::vector<std::size_t> tNodalIDs = designDomainNodeIDs();
    const auto tTimeStep = aTimeStep.hasValue() ? tsi::TimeStep{aTimeStep.valueOr(0.0)}
                                                : tsi::TimeStep{third_party_integration::stk_io::LastTimeStep{}};
    const auto tNodalField = third_party_integration::stk_io::read_nodal_field(filePath(), aFieldName, tTimeStep);
    return down_select_values_to_subset(tNodalField, tNodalIDs);
}

auto EntityRetrieval::nodalFields() const -> std::vector<std::string>
{
    return third_party_integration::stk_io::nodal_field_names(filePath());
}

}  // namespace plato::mesh
