#ifndef PLATO_FILTER_EXTENSION_COMMONINPUTVALIDATION
#define PLATO_FILTER_EXTENSION_COMMONINPUTVALIDATION

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <optional>
#include <string>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::filter::extension::detail
{
/// @brief Validate that filter radius is positive
template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
std::optional<std::string> validate_filter_radius_bounds(const FilterInput& aInput);

/// @brief Validate that filter radius is larger than the average of all element maximum edge lengths
template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
std::optional<std::string> validate_filter_radius_with_mesh(const FilterInput& aInput);

/// @brief Validate 'number_of_processors' an optional input is within bounds
template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
[[nodiscard]] std::optional<std::string> validate_number_of_processors(const FilterInput& aInput);

/// @brief Validate 'number_of_processors' is a common factor of comm world
template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
[[nodiscard]] std::optional<std::string> validate_number_of_processors_factor_of_comm_world(const FilterInput& aInput);

/// @brief Validate 'centering_type' is not empty
template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
[[nodiscard]] auto validate_filter_centering_type(const FilterInput& aInput) -> std::optional<std::string>;

template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
std::optional<std::string> validate_filter_radius_bounds(const FilterInput& aInput)
{
    namespace pfu = plato::utilities;
    return input_validation::error_message_for_parameter_out_of_bounds(input_parser::block_name<FilterInput>(),
                                                                       aInput.filter_radius, "filter_radius",
                                                                       pfu::lower_bounded(pfu::Exclusive{0.0}));
}

template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
std::optional<std::string> validate_filter_radius_with_mesh(const FilterInput& aInput,
                                                            const std::filesystem::path& aMeshFileName)
{
    if (!std::filesystem::exists(aMeshFileName))
    {
        return std::nullopt;  // Not our error
    }

    if (aInput.use_relative_radius && aInput.use_relative_radius.value())
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

template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
std::optional<std::string> validate_number_of_processors(const FilterInput& aInput)
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<FilterInput>(), aInput.number_of_processors, "number_of_processors",
        utilities::lower_bounded(utilities::Inclusive{1u}));
}

template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
std::optional<std::string> validate_number_of_processors_factor_of_comm_world(const FilterInput& aInput)
{
    const auto tRequestedRanks = aInput.number_of_processors.value_or(1u);
    const auto tTotalRanks = static_cast<std::size_t>(boost::mpi::communicator{}.size());
    if (tRequestedRanks > tTotalRanks)
    {
        return std::optional<std::string>{utilities::concatenate(
            "The number of MPI ranks requested for filter cannot exceed the available ranks for the entire run.\n",
            "The number of available ranks: ", tTotalRanks,
            ".\n The number of requested ranks for the filter: ", tRequestedRanks)};
    }
    if (tTotalRanks % tRequestedRanks != 0)
    {
        return std::optional<std::string>{
            utilities::concatenate("The number of MPI ranks requested for the filter has to be a factor of the "
                                   "available ranks for the entire run.\n",
                                   "The number of available ranks: ", tTotalRanks,
                                   ".\n The number of requested ranks for the filter: ", tRequestedRanks)};
    }

    return std::nullopt;
}

template <typename FilterInput>
    requires input_parser::InputBlockOfComponent<FilterInput, components::ComponentType::kFilter>
auto validate_filter_centering_type(const FilterInput& aInput) -> std::optional<std::string>
{
    if (!aInput.centering_type)
    {
        return input_validation::error_message_for_empty_parameter(input_parser::block_name<FilterInput>(),
                                                                   aInput.centering_type, "centering_type");
    }
    return std::nullopt;
}

}  // namespace plato::filter::extension::detail

#endif
