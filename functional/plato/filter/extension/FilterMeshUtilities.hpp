#ifndef PLATO_FILTER_EXTENSION_FILTERMESHUTILITIES
#define PLATO_FILTER_EXTENSION_FILTERMESHUTILITIES

#include <filesystem>

#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::filter::extension::detail
{
/// @brief Compute filter radius relative to average element maximum edge length if use_relative_radius is specified,
/// return the specified radius value otherwise
template <typename Filter>
double get_filter_radius(const Filter& aInput, const std::filesystem::path& aMeshFileName);

template <typename Filter>
double get_filter_radius(const Filter& aInput, const std::filesystem::path& aMeshFileName)
{
    if (aInput.use_relative_radius && aInput.use_relative_radius.value() == true)
    {
        const auto tBulk = third_party_integration::stk_io::read_mesh_bulk_data(aMeshFileName);
        const double tAverageLength = third_party_integration::stk_io::average_element_max_edge_length(*tBulk);
        return aInput.filter_radius.value() * tAverageLength;
    }
    return aInput.filter_radius.value();
}
}  // namespace plato::filter::extension::detail

#endif
