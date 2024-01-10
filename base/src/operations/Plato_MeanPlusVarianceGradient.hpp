/*
 //@HEADER
 // *************************************************************************
 //   Plato Engine v.1.0: Copyright 2018, National Technology & Engineering
 //                    Solutions of Sandia, LLC (NTESS).
 //
 // Under the terms of Contract DE-NA0003525 with NTESS,
 // the U.S. Government retains certain rights in this software.
 //
 // Redistribution and use in source and binary forms, with or without
 // modification, are permitted provided that the following conditions are
 // met:
 //
 // 1. Redistributions of source code must retain the above copyright
 // notice, this list of conditions and the following disclaimer.
 //
 // 2. Redistributions in binary form must reproduce the above copyright
 // notice, this list of conditions and the following disclaimer in the
 // documentation and/or other materials provided with the distribution.
 //
 // 3. Neither the name of the Sandia Corporation nor the names of the
 // contributors may be used to endorse or promote products derived from
 // this software without specific prior written permission.
 //
 // THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
 // EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 // IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 // PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
 // CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 // EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 // PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 // PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 // LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 // NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 // SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 //
 // Questions? Contact the Plato team (plato3D-help@sandia.gov)
 //
 // *************************************************************************
 //@HEADER
 */

/*
 * Plato_MeanPlusVarianceGradient.hpp
 *
 *  Created on: Jul 10, 2019
 */

#pragma once

#include "Plato_LocalOperation.hpp"
#include "Plato_StatisticsOperationsUtilities.hpp"

class PlatoApp;

namespace Plato
{

class InputData;

/******************************************************************************//**
 * @brief Compute mean plus standard deviation measure's gradient
**********************************************************************************/
class MeanPlusVarianceGradient : public Plato::LocalOp
{
public:
    MeanPlusVarianceGradient() = default;
    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aPlatoApp PLATO application interface
     * @param [in] aOperationNode local operation input data
    **********************************************************************************/
    MeanPlusVarianceGradient(PlatoApp* aPlatoApp, const Plato::InputData& aOperationNode);

    /******************************************************************************//**
     * @brief Destructor
    **********************************************************************************/
    ~MeanPlusVarianceGradient() override;

    /******************************************************************************//**
     * @brief Perform local operation - compute mean plus standard deviation measure's gradient
    **********************************************************************************/
    void operator()() override;

    /******************************************************************************//**
     * @brief Get the input and output arguments associated with the local operation
     * @param [in/out] aLocalArgs local input and output arguments
    **********************************************************************************/
    void getArguments(std::vector<LocalArg>& aLocalArgs) override;

    /******************************************************************************//**
     * @brief Return name used by the user to identify the function.
     * @return user-defined function name
    **********************************************************************************/
    std::string getOperationName() const;

    /******************************************************************************//**
     * @brief Return function's identifier, i.e. MeanPlusStdDev.
     * @return function's identifier, the user shall use "MeanPlusStdDev" as the
     *   function identifier. the function identifier is used by the PLATO application
     *   to create the function.
    **********************************************************************************/
    std::string getFunctionIdentifier() const;

    /******************************************************************************//**
     * @brief Return criteria's data layout.
     * return data layout, the only valid option is scalar value
    **********************************************************************************/
    Plato::data::layout_t getCriterionValueDataLayout() const;

    /******************************************************************************//**
     * @brief Return criteria gradient's data layout.
     * return data layout, valid options are element and node field
    **********************************************************************************/
    Plato::data::layout_t getCriterionGradientDataLayout() const;

    /******************************************************************************//**
     * @brief Return list of standard deviation multipliers in ascending order
     * @return standard deviation multipliers
    **********************************************************************************/
    double getStandardDeviationMultipliers() const;

    /******************************************************************************//**
     * @brief Return the probability associated with a given criteria value sample
     * @param [in] aInput criteria value sample's argument name
     * @return probability
    **********************************************************************************/
    double getCriterionValueProbability(const std::string& aInput) const;

    /******************************************************************************//**
     * @brief Return the probability associated with a given criteria gradient sample
     * @param [in] aInput criteria gradient sample's argument name
     * @return probability
    **********************************************************************************/
    double getCriterionGradientProbability(const std::string& aInput) const;

    /******************************************************************************//**
     * @brief Return output argument associated with a given criterion value sample
     * @param [in] aInput output argument name
     * @return output argument name
    **********************************************************************************/
    std::string getCriterionValueOutputArgument(const std::string& aInput) const;

    /******************************************************************************//**
     * @brief Return output argument associated with a given criterion gradient sample
     * @param [in] aInput output argument name
     * @return output argument name
    **********************************************************************************/
    std::string getCriterionGradientOutputArgument(const std::string& aInput) const;

private:
    /******************************************************************************//**
     * @brief Initialize mean plus variance local operation
     * @param [in] aOperationNode operation input data
    **********************************************************************************/
    void initialize(const Plato::InputData& aOperationNode);

    /******************************************************************************//**
     * @brief Check that the criterion value and criterion gradient sample probabilities match.
    **********************************************************************************/
    void checkInputProbabilityValues();

    /******************************************************************************//**
     * @brief Parse user defined function name,. Use the default value, Stochastic Criterion
     *   Gradient, if the user does not provide a name.
     * @param [in] aOperationNode operation input data
    **********************************************************************************/
    void parseName(const Plato::InputData& aOperationNode);

    /******************************************************************************//**
     * @brief Parse function identifier
     * @param [in] aOperationNode operation input data
    **********************************************************************************/
    void parseFunction(const Plato::InputData& aOperationNode);

    /******************************************************************************//**
     * @brief Set criterion value sample-probability pairs
    **********************************************************************************/
    void setCriterionValueSampleProbabilityPairs();

    /******************************************************************************//**
     * @brief Set criterion gradient sample-probability pairs
    **********************************************************************************/
    void setCriterionGradientSampleProbabilityPairs();

    /******************************************************************************//**
     * @brief Set criterion gradient sample-probability pairs (node-based data)
    **********************************************************************************/
    void setNodeFieldGradientSampleProbabilityPairs();

    /******************************************************************************//**
     * @brief Set criterion gradient sample-probability pairs (element-based data)
    **********************************************************************************/
    void setElementFieldGradientSampleProbabilityPairs();

    /******************************************************************************//**
     * @brief Increase the capacity of the criterion gradient's set of sample-probability
     *    pairs to the number of samples.
    **********************************************************************************/
    void reserveCriterionGradSampleProbabilityPairs();

    /******************************************************************************//**
     * @brief Increase the capacity of the criterion value's set of sample-probability
     *    pairs to the number of samples.
    **********************************************************************************/
    void reserveCriterionValueSampleProbabilityPairs();

    /******************************************************************************//**
     * @brief Parse data layout from input file
     * @param [in] aOperationNode operation input data
     * @param [in] aCriteriaName criteria name
     **********************************************************************************/
    Plato::data::layout_t parseDataLayout(const Plato::InputData& aOperationNode, const std::string& aCriteriaName);

    /******************************************************************************//**
     * @brief Return pointer to output data associated with the criterion gradient
     * @param [in] aStatisticMeasure string statistic measure
     * @return pointer to output data
    **********************************************************************************/
    double* getCriterionGradientOutputData(const std::string& aStatisticMeasure);

    /******************************************************************************//**
     * @brief Return pointer to output data associated with the criterion value
     * @param [in] aStatisticMeasure string statistic measure
     * @return pointer to output data
    **********************************************************************************/
    std::vector<double>* getCriterionValueOutputData(const std::string& aStatisticMeasure);

    /******************************************************************************//**
     * @brief Parse input arguments from XML files
     * @param [in] aDataLayout data layout for input sample set
     * @param [in] aInput input arguments
     * @param [out] aOutput input argument name to probability map
    **********************************************************************************/
    void parseInputs(const Plato::data::layout_t& aDataLayout,
                     const Plato::InputData& aInput,
                     std::map<std::string, double> & aOutput);

    /******************************************************************************//**
     * @brief Parse criteria value input arguments
     * @param [in] aOperationNode operation input data
    **********************************************************************************/
    void parseCriterionValueArguments(const Plato::InputData& aOperationNode);

    /******************************************************************************//**
     * @brief Parse criteria gradient input arguments
     * @param [in] aOperationNode operation input data
    **********************************************************************************/
    void parseCriterionGradientArguments(const Plato::InputData& aOperationNode);

    /******************************************************************************//**
     * @brief Return statistic measure, e.g. 'MEAN', 'STD_DEV' or 'MEAN_PLUS_1_STD_DEV'
     * @param [in] aOutputNode XML input data associated with the output arguments
     * @param [in] aOutputArgumentName output argument name
     * @return statistic measure string
    **********************************************************************************/
    std::string getStatisticMeasure(const Plato::InputData& aOutputNode, const std::string& aOutputArgumentName) const;

    /******************************************************************************//**
     * @brief Return output argument
     * @param [in] aOutputNode XML input data associated with the output arguments
     * @param [in] aCriterionName criterion block name, e.g. CriterionValue
     * @return output argument
    **********************************************************************************/
    std::string getOutputArgument(const Plato::InputData& aOutputNode, const std::string& aCriterionName) const;

    /******************************************************************************//**
     * @brief Parse criteria value output arguments
     * @param [in] aCriteriaNode XML input data associated with a given criteria node
    **********************************************************************************/
    void parseCriterionValueOutputs(const Plato::InputData& aCriteriaNode);

    /******************************************************************************//**
     * @brief Parse criteria gradient output arguments
     * @param [in] aCriteriaNode XML input data associated with a given criteria node
    **********************************************************************************/
    void parseCriterionGradientOutputs(const Plato::InputData& aCriteriaNode);

    /******************************************************************************//**
     * @brief Set standard deviation multiplier
     * @param [in] aStatisticMeasure string with statistic measure, e.g. 'MEAN_PLUS_1_STD_DEV'
    **********************************************************************************/
    void setStandardDeviationMultiplier(const std::string& aStatisticMeasure);

    /******************************************************************************//**
     * @brief Return numeric value for the probability
     * @param [in] aTokens list of tokens
     * @return numeric value for the probability
    **********************************************************************************/
    double getStandardDeviationMultiplier(const std::vector<std::string>& aTokens);

    /******************************************************************************//**
     * @brief Return sample probability
     * @param [in] aInputNode XML inputs associated with a given input argument
     * @return sample probability
    **********************************************************************************/
    double getProbability(const Plato::InputData& aInputNode);

    /******************************************************************************//**
     * @brief Add a new local argument to the set of local arguments
     * @param [in] aDataLayout data layout, e.g. SCALAR, NODE FIELD, ELEMENT FIELD
     * @param [in] aArgumentName argument name
    **********************************************************************************/
    void addLocalArgument(const Plato::data::layout_t& aDataLayout, const std::string & aArgumentName);

    /******************************************************************************//**
     * @brief Compute criterion gradient sample set's mean
    **********************************************************************************/
    void computeMeanCriterionGradientSampleSet();

    /******************************************************************************//**
     * @brief Compute criterion value sample set's mean and standard deviation
    **********************************************************************************/
    void computeMeanAndStdDevCriterionValueSampleSet();

    /******************************************************************************//**
     * @brief Compute the gradient of the mean plus standard deviation criterion
    **********************************************************************************/
    void computeGradientMeanPlusStandardDeviationCriterion();

    /******************************************************************************//**
     * @brief Compute the gradient of the mean plus standard deviation criterion -
     *     node field gradient type
    **********************************************************************************/
    void computeGradientMeanPlusStdDevCriterionForNodeField();

    /******************************************************************************//**
     * @brief Compute the gradient of the mean plus standard deviation criterion -
     *     element field gradient type
    **********************************************************************************/
    void computeGradientMeanPlusStdDevCriterionForElementField();

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
      aArchive & boost::serialization::make_nvp("LocalOp",boost::serialization::base_object<LocalOp>(*this));
      aArchive & boost::serialization::make_nvp("OperationName",mOperationName);
    }

private:
    bool mIsMeanPlusStdDevFormulation; /*!< flag is set to true if it is a robust optimization problem */

    double mStdDevMultiplier; /*!< standard deviation multiplier */
    std::string mOperationName; /*!< user defined function name */
    std::string mFunctionIdentifier; /*!< function identifier */
    std::string mOutputGradientArgumentName; /*!< output criterion gradient argument name */

    Plato::data::layout_t mCriterionValueDataLayout; /*!< criteria value data layout */
    Plato::data::layout_t mCriterionGradientDataLayout; /*!< criteria gradient data layout */

    std::vector<Plato::LocalArg> mLocalArguments; /*!< input/output shared data set */
    std::vector<Plato::SampleProbPair<double, double>> mCriterionValueSamplesToProbability; /*!< criterion value samples to probability pairs */
    std::vector<Plato::SampleProbPair<double*, double>> mCriterionGradSamplesToProbability; /*!< criterion gradient samples to probability pairs */

    std::map<std::string, double> mCriterionValueSamplesArgNameToProbability; /*!< criterion value samples to probability map */
    std::map<std::string, double> mCriterionGradSamplesArgNameToProbability; /*!< criterion gradient samples to probability map */

    std::map<std::string, std::string> mCriterionValueStatisticsToOutputName; /*!< criterion value statistics to output argument name map */
    std::map<std::string, std::string> mCriterionGradientStatisticsToOutputName; /*!< criterion gradient statistics to output argument name map */
};
// class MeanPlusVarianceGradient

}
// namespace Plato

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY2(Plato::MeanPlusVarianceGradient, "MeanPlusVarianceGradient")