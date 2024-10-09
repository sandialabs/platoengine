#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

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

}  // namespace

auto read_nodal_field_as_vector(const std::filesystem::path& aMeshName, std::string_view aFieldName)
    -> std::vector<double>
{
    const auto tField = stk_io::read_nodal_field(aMeshName, aFieldName);
    return convert_map_sorted_vector(tField);
}

auto read_element_field_as_vector(const std::filesystem::path& aMeshName, std::string_view aFieldName)
    -> std::vector<double>
{
    const auto tField = stk_io::read_element_field(aMeshName, aFieldName);
    return convert_map_sorted_vector(tField);
}

}  // namespace plato::third_party_integration::stk_io::test_utilities
