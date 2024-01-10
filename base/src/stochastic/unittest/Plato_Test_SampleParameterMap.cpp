#include <gtest/gtest.h>

#include "Plato_SampleParameterMap.hpp"
#include "Plato_StochasticSampleSharedDataNames.hpp"

#include <unordered_map>

namespace
{
Plato::StochasticSampleSharedDataNames test_data()
{
 return Plato::StochasticSampleSharedDataNames{
        /*.mOutputValueSharedDataName = */ "value",
        /*.mOutputGradientSharedDataName = */ "gradient",
        /*.mParameters = */ 
        {
            Plato::ParameterAndDistribution{
                /*.mParameterName = */ "parameter_1",
                /*.mDistributionName = */ "distribution_1"
            },
            Plato::ParameterAndDistribution{
                /*.mParameterName = */ "parameter_2",
                /*.mDistributionName = */ "distribution_2"
            }

        }
    };
}
}

TEST(SampleParameterMap, GenerateMapValue)
{
    const Plato::StochasticSampleSharedDataNames tTestData = test_data();
    const std::unordered_map<std::string, unsigned int> tDistributions = {{"distribution_2", 0}, {"distribution_1", 1}};
    const Plato::SampleParameterMap tDataMap = 
        Plato::shared_data_parameter_map_for_value(tTestData, tDistributions);

    EXPECT_EQ(tDataMap.mOutputSharedDataName, "value");
    EXPECT_EQ(tDataMap.mParameterIndexToParameterName.size(), 2);
    EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(0), "parameter_2");
    EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(1), "parameter_1");
}

TEST(SampleParameterMap, GenerateMapGradient)
{
    const Plato::StochasticSampleSharedDataNames tTestData = test_data();
    const std::unordered_map<std::string, unsigned int> tDistributions = {{"distribution_1", 0}, {"distribution_2", 1}};
    const Plato::SampleParameterMap tDataMap = 
        Plato::shared_data_parameter_map_for_gradient(tTestData, tDistributions);

    EXPECT_EQ(tDataMap.mOutputSharedDataName, "gradient");
    EXPECT_EQ(tDataMap.mParameterIndexToParameterName.size(), 2);
    EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(0), "parameter_1");
    EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(1), "parameter_2");
}

TEST(SampleParameterMap, GenerateValueMaps)
{
    const std::unordered_map<std::string, unsigned int> tDistributions = {{"distribution_2", 0}, {"distribution_1", 1}};
    const std::vector tDataMaps = 
        Plato::shared_data_parameter_maps_for_value({test_data(), test_data()}, tDistributions);

    ASSERT_EQ(tDataMaps.size(), 2);
    for(const auto& tDataMap : tDataMaps)
    {
        EXPECT_EQ(tDataMap.mOutputSharedDataName, "value");
        EXPECT_EQ(tDataMap.mParameterIndexToParameterName.size(), 2);
        EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(0), "parameter_2");
        EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(1), "parameter_1");
    }
}

TEST(SampleParameterMap, GenerateGradientMaps)
{
    const std::unordered_map<std::string, unsigned int> tDistributions = {{"distribution_1", 0}, {"distribution_2", 1}};
    const std::vector tDataMaps = 
        Plato::shared_data_parameter_maps_for_gradient({test_data(), test_data()}, tDistributions);

    ASSERT_EQ(tDataMaps.size(), 2);
    for(const auto& tDataMap : tDataMaps)
    {
        EXPECT_EQ(tDataMap.mOutputSharedDataName, "gradient");
        EXPECT_EQ(tDataMap.mParameterIndexToParameterName.size(), 2);
        EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(0), "parameter_1");
        EXPECT_EQ(tDataMap.mParameterIndexToParameterName.at(1), "parameter_2");
    }
}

TEST(SampleParameterMap, DistributionMap)
{
    const Plato::StochasticSampleSharedDataNames tTestData = test_data();
    const std::unordered_map<std::string, unsigned int> tDistributionMap = Plato::distribution_map(tTestData);

    EXPECT_EQ(tDistributionMap.at("distribution_1"), 0);
    EXPECT_EQ(tDistributionMap.at("distribution_2"), 1);
}
