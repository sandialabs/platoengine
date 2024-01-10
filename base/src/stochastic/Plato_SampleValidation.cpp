#include "Plato_SampleValidation.hpp"

#include "Plato_Interface.hpp"
#include "Plato_StochasticSampleSharedDataNames.hpp"

#include <Teuchos_ParameterList.hpp>

#include <unordered_set>

namespace Plato
{
namespace
{
std::unordered_set<std::string> distribution_names(
    const std::vector<Plato::ParameterAndDistribution>& aParameterAndDistributionNames)
{
    std::unordered_set<std::string> tDistributions;
    for(const Plato::ParameterAndDistribution& tParameter : aParameterAndDistributionNames)
    {
        tDistributions.insert(tParameter.mDistributionName);
    }
    return tDistributions;
}

bool distributions_match( 
    const Plato::StochasticSampleSharedDataNames& tSampleDatum,
    const std::unordered_set<std::string>& tDistributionsToMatch )
{
    const std::unordered_set<std::string> tDistributionNames = distribution_names(tSampleDatum.mParameters);
    return tDistributionNames == tDistributionsToMatch;
}
}

std::optional<std::string> are_distributions_consistent_for_all_samples(
    const std::vector<StochasticSampleSharedDataNames>& aSampleData)
{
    if(aSampleData.empty())
    {
        return "No stochastic sample data was found in the input under Optimizer/OptimizationVariables.";
    }
    // Grab all distribution names from first sample and check that they're unique.
    const std::unordered_set<std::string> tDistributionNames = 
        distribution_names(aSampleData.front().mParameters);
    if(tDistributionNames.size() != aSampleData.front().mParameters.size())
    {
        return "Expected unique distributions in each sample.";
    }
    // Check that remaining samples have all the same distributions defined.
    for(const Plato::StochasticSampleSharedDataNames& tSampleDatum : aSampleData)
    {
        if(!distributions_match(tSampleDatum, tDistributionNames))
        {
            return "Mismatch in distributions defined on all samples.";
        }
    }
    return std::nullopt;
}

std::optional<std::string> undefined_distributions(
    const std::vector<StochasticSampleSharedDataNames>& aSampleData,
    const Teuchos::ParameterList& aDistributionParameterList)
{
    for(const auto& tSampleData : aSampleData)
    {
        for(const auto& tParameter : tSampleData.mParameters)
        {
            if(!aDistributionParameterList.isSublist(tParameter.mDistributionName))
            {   
                return tParameter.mDistributionName;
            }
        }
    }
    return std::nullopt;
}

std::optional<std::string> undefined_parameters(
    const std::vector<StochasticSampleSharedDataNames>& aSampleData,
    const Plato::Interface& aInterface )
{
    for(const auto& tSampleData : aSampleData)
    {
        for(const auto& tParameter : tSampleData.mParameters)
        {
            if(!aInterface.dataLayerHasParameter(tParameter.mParameterName))
            {
                return tParameter.mParameterName;
            }
        }
    }
    return std::nullopt;
}

}
