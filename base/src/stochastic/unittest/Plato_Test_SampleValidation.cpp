#include <gtest/gtest.h>

#include "Plato_SampleValidation.hpp"
#include "Plato_StochasticSampleSharedDataNames.hpp"

#include <Teuchos_ParameterList.hpp>

TEST(SampleValidation, Empty)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData;
    EXPECT_TRUE(Plato::are_distributions_consistent_for_all_samples(tSampleData).has_value());
}

TEST(SampleValidation, ValidOneEntry)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
        }
    };
    EXPECT_FALSE(Plato::are_distributions_consistent_for_all_samples(tSampleData).has_value());
}

TEST(SampleValidation, InvalidOneEntry)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
                /*.mDistributionName = */ "distribution_1"
            }
        }
        }
    };
    EXPECT_TRUE(Plato::are_distributions_consistent_for_all_samples(tSampleData).has_value());
}

TEST(SampleValidation, ValidTwoEntries)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
        },
        Plato::StochasticSampleSharedDataNames{
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
        }
    };
    EXPECT_FALSE(Plato::are_distributions_consistent_for_all_samples(tSampleData).has_value());
}

TEST(SampleValidation, InvalidTwoEntriesWrongSize)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
        },
        Plato::StochasticSampleSharedDataNames{
        /*.mOutputValueSharedDataName = */ "value",
        /*.mOutputGradientSharedDataName = */ "gradient",
        /*.mParameters = */ 
        {
            Plato::ParameterAndDistribution{
                /*.mParameterName = */ "parameter_1",
                /*.mDistributionName = */ "distribution_1"
            }
        }
        }
    };
    EXPECT_TRUE(Plato::are_distributions_consistent_for_all_samples(tSampleData).has_value());
}

TEST(SampleValidation, InvalidTwoEntriesWrongNames)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
        },
        Plato::StochasticSampleSharedDataNames{
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
                /*.mDistributionName = */ "distribution_3"
            }
        }
        }
    };
    EXPECT_TRUE(Plato::are_distributions_consistent_for_all_samples(tSampleData).has_value());
}

TEST(UndefinedDistributions, AllDefined)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
        },
        Plato::StochasticSampleSharedDataNames{
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
                /*.mDistributionName = */ "distribution_3"
            }
        }
        }
    };
    Teuchos::ParameterList tDistributions;
    tDistributions.sublist("distribution_1");
    tDistributions.sublist("distribution_2");
    tDistributions.sublist("distribution_3");
    EXPECT_FALSE(Plato::undefined_distributions(tSampleData, tDistributions).has_value());
}

TEST(UndefinedDistributions, OneUndefined)
{
    const std::vector<Plato::StochasticSampleSharedDataNames> tSampleData = 
    {
        Plato::StochasticSampleSharedDataNames{
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
                /*.mDistributionName = */ "distribution_3"
            }
        }
        }
    };
    Teuchos::ParameterList tDistributions;
    tDistributions.sublist("distribution_1");
    tDistributions.sublist("distribution_2");
    ASSERT_TRUE(Plato::undefined_distributions(tSampleData, tDistributions).has_value());
    EXPECT_EQ(Plato::undefined_distributions(tSampleData, tDistributions).value(), "distribution_3");
}
