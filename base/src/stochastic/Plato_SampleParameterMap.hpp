#pragma once

#include "Plato_StochasticSampleSharedDataNames.hpp"

#include <string>
#include <unordered_map>

namespace Plato
{
/// @brief Maps SharedData parameter names to ROL parameter indices.
///
/// This is used to determine which SharedData object holds the results of
/// the call to the compute function, and also maps the parameter index 
/// (held by ROL in a vector) to the SharedData parameter name.
struct SampleParameterMap
{
    std::string mOutputSharedDataName;
    std::unordered_map<unsigned int, std::string> mParameterIndexToParameterName;
};

/// @brief Creates a SampleParameterMap using the mOutputValueSharedDataName field of StochasticSampleSharedDataNames
/// @param aSampleSharedData SharedData parsed from the input 
/// @param aDistributionMap Maps the probability distribution name to its parameter index in ROL. This should match
///  the distribution's position in the vector of ROL::Distribution used to initialize the ROL::SampleGenerator.
/// @throw std::out_of_range Throws if a distribution name in @a aSampleSharedData is not found in @a aDistributionMap
[[nodiscard]] Plato::SampleParameterMap shared_data_parameter_map_for_value(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap);

/// @brief Creates a vector of SampleParameterMap using the mOutputValueSharedDataName field of StochasticSampleSharedDataNames
/// @sa shared_data_parameter_map_for_value
[[nodiscard]] std::vector<Plato::SampleParameterMap> shared_data_parameter_maps_for_value(
    const std::vector<Plato::StochasticSampleSharedDataNames>& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap);

/// @brief Creates a SampleParameterMap using the mOutputGradientSharedDataName field of StochasticSampleSharedDataNames
/// @param aSampleSharedData SharedData parsed from the input 
/// @param aDistributionMap Maps the probability distribution name to its parameter index in ROL. This should match
///  the distribution's position in the vector of ROL::Distribution used to initialize the ROL::SampleGenerator.
/// @throw std::out_of_range Throws if a distribution name in @a aSampleSharedData is not found in @a aDistributionMap
[[nodiscard]] Plato::SampleParameterMap shared_data_parameter_map_for_gradient(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap);

/// @brief Creates a vector of SampleParameterMap using the mOutputValueSharedDataName field of StochasticSampleSharedDataNames
/// @sa shared_data_parameter_map_for_gradient
[[nodiscard]] std::vector<Plato::SampleParameterMap> shared_data_parameter_maps_for_gradient(
    const std::vector<Plato::StochasticSampleSharedDataNames>& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap);

/// @brief Creates a map between distribution names contained in the `mParameterAndDistribution` field of
///  @a aSampleSharedData. 
///
/// This is used to achieve a consistent mapping for all distributions/parameters in 
/// StochasticSampleSharedDataNames objects for evaluation.
/// @note The set of StochasticSampleSharedDataNames objects should be validated with 
/// are_distributions_consistent_for_all_samples and undefined_distributions to ensure a consistent mapping.
[[nodiscard]] std::unordered_map<std::string, unsigned int> distribution_map(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData);
}
