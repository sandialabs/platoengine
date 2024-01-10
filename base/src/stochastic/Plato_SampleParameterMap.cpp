#include "Plato_SampleParameterMap.hpp"

#include <cassert>
#include <algorithm>

namespace Plato
{

namespace
{
auto parameter_map(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap) -> std::unordered_map<unsigned int, std::string> 
{
    std::unordered_map<unsigned int, std::string> tParameterMap;
    std::transform(
        aSampleSharedData.mParameters.cbegin(), aSampleSharedData.mParameters.cend(), std::inserter(tParameterMap, tParameterMap.end()),
        [&aDistributionMap](const Plato::ParameterAndDistribution& aParameterAndDistribution) -> std::pair<unsigned int, std::string>
        {
            const auto tIndex = aDistributionMap.at(aParameterAndDistribution.mDistributionName);
            return {tIndex, aParameterAndDistribution.mParameterName};
        }
    );
    return tParameterMap;
}

}

Plato::SampleParameterMap shared_data_parameter_map_for_value(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap)
{
    return Plato::SampleParameterMap{
        /*.mOutputSharedDataName = */ aSampleSharedData.mOutputValueSharedDataName,
        /*.mParameterIndexToParameterName = */ parameter_map(aSampleSharedData, aDistributionMap)
    };
}

std::vector<Plato::SampleParameterMap> shared_data_parameter_maps_for_value(
    const std::vector<Plato::StochasticSampleSharedDataNames>& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap)
{
    std::vector<Plato::SampleParameterMap> tParameterMaps;
    std::transform(aSampleSharedData.cbegin(), aSampleSharedData.cend(), std::back_inserter(tParameterMaps),
    [&aDistributionMap](const Plato::StochasticSampleSharedDataNames& aSampleSharedDataNames)
    {
        return shared_data_parameter_map_for_value(aSampleSharedDataNames, aDistributionMap);
    });
    return tParameterMaps;
}

Plato::SampleParameterMap shared_data_parameter_map_for_gradient(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap)
{
    return Plato::SampleParameterMap{
        /*.mOutputSharedDataName = */ aSampleSharedData.mOutputGradientSharedDataName,
        /*.mParameterIndexToParameterName = */ parameter_map(aSampleSharedData, aDistributionMap)
    };
}

std::vector<Plato::SampleParameterMap> shared_data_parameter_maps_for_gradient(
    const std::vector<Plato::StochasticSampleSharedDataNames>& aSampleSharedData,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap)
{
    std::vector<Plato::SampleParameterMap> tParameterMaps;
    std::transform(aSampleSharedData.cbegin(), aSampleSharedData.cend(), std::back_inserter(tParameterMaps),
    [&aDistributionMap](const Plato::StochasticSampleSharedDataNames& aSampleSharedDataNames)
    {
        return shared_data_parameter_map_for_gradient(aSampleSharedDataNames, aDistributionMap);
    });
    return tParameterMaps;
}

std::unordered_map<std::string, unsigned int> distribution_map(
    const Plato::StochasticSampleSharedDataNames& aSampleSharedData)
{
    std::unordered_map<std::string, unsigned int> tDistributionMap;
    std::transform(
        aSampleSharedData.mParameters.cbegin(), 
        aSampleSharedData.mParameters.cend(), 
        std::inserter(tDistributionMap, tDistributionMap.end()),
        [index = 0](const Plato::ParameterAndDistribution& aParameterAndDistribution) mutable {
            return std::make_pair(aParameterAndDistribution.mDistributionName, index++);
        });
    return tDistributionMap;
}
}
