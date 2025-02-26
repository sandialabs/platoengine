#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

#include <map>
#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
namespace
{

[[nodiscard]] auto convert_map_sorted_vector(const std::map<std::size_t, double>& aMap)
{
    std::vector<double> tVector;
    tVector.reserve(aMap.size());
    std::transform(aMap.begin(), aMap.end(), std::back_inserter(tVector),
                   [](const auto& aPair) { return aPair.second; });
    return tVector;
}

template <stk::topology::rank_t Rank>
void write_mesh_scalar_field_impl(const std::filesystem::path& aInputMeshName,
                                  const ScalarFieldFunction& aScalarField,
                                  const std::string_view aFieldName,
                                  const std::filesystem::path& aOutputMeshName)
{
    auto tIOBroker = create_io_broker_from_input_file(aInputMeshName);
    const auto tFileHandle = create_output_mesh(aOutputMeshName, *tIOBroker);
    if constexpr (Rank == stk::topology::ELEM_RANK)
    {
        stk_io::initialize_element_scalar_field(*tIOBroker, aFieldName);
        stk_io::populate_element_scalar_field_values(*tIOBroker, aFieldName, aScalarField);
        stk_io::add_element_field_to_output_file(*tIOBroker, tFileHandle, aFieldName);
    }
    else
    {
        stk_io::initialize_nodal_scalar_field(*tIOBroker, aFieldName);
        stk_io::populate_nodal_scalar_field_values(*tIOBroker, aFieldName, aScalarField);
        stk_io::add_nodal_field_to_output_file(*tIOBroker, tFileHandle, aFieldName);
    }
    write_fields_at_time(*tIOBroker, tFileHandle, /*aOutputTime=*/1.0);
}

}  // namespace

auto read_nodal_field_as_vector(const std::filesystem::path& aMeshName, std::string_view aFieldName, const double aTime)
    -> std::vector<double>
{
    const auto tField = stk_io::read_nodal_field(aMeshName, aFieldName, aTime);
    return convert_map_sorted_vector(tField);
}

auto read_element_field_as_vector(const std::filesystem::path& aMeshName,
                                  std::string_view aFieldName,
                                  const double aTime) -> std::vector<double>
{
    const auto tField = stk_io::read_element_field(aMeshName, aFieldName, aTime);
    return convert_map_sorted_vector(tField);
}

void write_nodal_scalar_field(const std::filesystem::path& aInputMeshName,
                              const ScalarFieldFunction& aScalarField,
                              const std::string_view aFieldName,
                              const std::filesystem::path& aOutputMeshName)
{
    write_mesh_scalar_field_impl<stk::topology::NODE_RANK>(aInputMeshName, aScalarField, aFieldName, aOutputMeshName);
}

void write_element_scalar_field(const std::filesystem::path& aInputMeshName,
                                const ScalarFieldFunction& aScalarField,
                                const std::string_view aFieldName,
                                const std::filesystem::path& aOutputMeshName)
{
    write_mesh_scalar_field_impl<stk::topology::ELEM_RANK>(aInputMeshName, aScalarField, aFieldName, aOutputMeshName);
}

}  // namespace plato::third_party_integration::stk_io::test_utilities
