#pragma once

#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>

namespace Plato
{
struct ParameterAndDistribution
{
    std::string mParameterName;
    std::string mDistributionName;

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
      aArchive & boost::serialization::make_nvp("ParameterName", mParameterName);
      aArchive & boost::serialization::make_nvp("DistributionName", mDistributionName);
    }
};

struct StochasticSampleSharedDataNames
{
    std::string mOutputValueSharedDataName;
    std::string mOutputGradientSharedDataName;
    std::vector<ParameterAndDistribution> mParameters;

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
      aArchive & boost::serialization::make_nvp("OutputValueSharedDataName", mOutputValueSharedDataName);
      aArchive & boost::serialization::make_nvp("OutputGradientSharedDataName", mOutputGradientSharedDataName);
      aArchive & boost::serialization::make_nvp("Parameters", mParameters);
    }
};

}
