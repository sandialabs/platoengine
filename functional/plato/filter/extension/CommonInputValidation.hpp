#ifndef PLATO_FILTER_EXTENSION_COMMONINPUTVALIDATION
#define PLATO_FILTER_EXTENSION_COMMONINPUTVALIDATION

#include <optional>
#include <string>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/third_party_integration/stk_io/IOUtilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::filter::extension::detail
{
/// @brief Validate that filter radius is positive
template <typename Filter>
std::optional<std::string> validate_filter_radius_bounds(const Filter& aInput);

/// @brief Validate that filter radius is larger than the average of all element maximum edge lengths
template <typename Filter>
std::optional<std::string> validate_filter_radius_with_mesh(const Filter& aInput);

template <typename Filter>
std::optional<std::string> validate_filter_radius_bounds(const Filter& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_parameter_out_of_bounds(input_parser::block_name<Filter>(), aInput.filter_radius,
                                                           "filter_radius", pfu::lower_bounded(pfu::Exclusive{0.0}));
}

template <typename Filter>
std::optional<std::string> validate_filter_radius_with_mesh(const Filter& aInput,
                                                            const std::filesystem::path& aMeshFileName)
{
    if (aInput.use_relative_radius && aInput.use_relative_radius.value() == true)
    {
        return std::nullopt;
    }

    const auto tBulk = third_party_integration::stk_io::read_mesh_bulk_data(aMeshFileName);
    const double tAverageLength = third_party_integration::stk_io::average_element_max_edge_length(*tBulk);
    if (aInput.filter_radius && aInput.filter_radius <= tAverageLength)
    {
        return utilities::concatenate("The specified filter radius (", std::to_string(aInput.filter_radius.value()),
                                      ") is smaller than the average element edge length in the mesh (",
                                      std::to_string(tAverageLength), ")");
    }
    return std::nullopt;
}

}  // namespace plato::filter::extension::detail

#endif
