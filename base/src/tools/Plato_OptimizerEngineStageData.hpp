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
 * Plato_OptimizerEngineStageData.hpp
 *
 *  Created on: Oct 27, 2017
 */

#ifndef PLATO_OPTIMIZERENGINESTAGEDATA_HPP_
#define PLATO_OPTIMIZERENGINESTAGEDATA_HPP_

#include <map>
#include <string>
#include <vector>

#include "Plato_InputData.hpp"
#include "Plato_StochasticSampleSharedDataNames.hpp"

namespace Plato
{

class OptimizerEngineStageData : public Plato::InputData
{
public:
    void setCheckGradient(const bool aInput);
    void setCheckHessian(const bool aInput);
    void setUserInitialGuess(const bool aInput);
    void setOutputControlToFile(const bool aInput);
    void setOutputDiagnosticsToFile(const bool aInput);

    /******************************************************************************//**
     * @brief Enable mean norm calculations, yes = true or no = false
     * @param [in] aInput flag
    ***********************************************************************************/
    void setMeanNorm(const bool aInput);

    /******************************************************************************//**
     * @brief Return mean norm flag
     * @return yes = true or no = false
    ***********************************************************************************/
    bool getMeanNorm() const;
    bool getCheckGradient() const;
    bool getCheckHessian() const;
    bool getUserInitialGuess() const;
    bool getOutputControlToFile() const;
    bool getOutputDiagnosticsToFile() const;

    /******************************************************************************//**
     * @brief Numerical method used to compute application of vector to Hessian operator
     * @param [in] aInput numerical method
    ***********************************************************************************/
    void setHessianType(const std::string & aInput);

    /******************************************************************************//**
     * @brief Return numerical method used to compute application of vector to Hessian operator
     * @return numerical method
    ***********************************************************************************/
    std::string getHessianType() const;

    std::vector<double> getLowerBoundValues() const;
    std::vector<double> getUpperBoundValues() const;
    void setLowerBoundValues(const std::vector<double> & aInput);
    void setUpperBoundValues(const std::vector<double> & aInput);

    size_t getMaxNumIterations() const;
    void setMaxNumIterations(const size_t aInput);

    std::vector<double> getInitialGuess() const;
    void setInitialGuess(const std::vector<double> & aInput);
    void setInitialGuess(const std::vector<std::string> & aInput);

    double getROLPerturbationScale() const;
    void setROLPerturbationScale(const double aInput);
    int getROLCheckGradientSteps() const;
    void setROLCheckGradientSteps(const int aInput);
    int getROLCheckGradientStepSize() const;
    void setROLCheckGradientStepSize(const int aInput);
    int getROLCheckGradientSeed() const;
    void setROLCheckGradientSeed(const int aInput);

    const std::string& getROLStochasticDistributionsFile() const;
    void setROLStochasticDistributionsFile(std::string aInput);
    int getROLStochasticNumberOfSamples() const;
    void setROLStochasticNumberOfSamples(int aInput);
    int getROLStochasticSamplerSeed() const;
    void setROLStochasticSamplerSeed(int aInput);

    std::string getStateName() const;
    void setStateNames(const std::string & aInput);

    std::string getInputFileName() const;
    void setInputFileName(const std::string & aInput);

    size_t getNumControlVectors() const;
    std::vector<std::string> getControlNames() const;
    std::string getControlName(const size_t aIndex) const;
    void addControlName(const std::string & aInput);
    void setControlNames(const std::vector<std::string> & aInput);

    void addFilteredControlName(const std::string & aInput);
    std::string getFilteredControlName(const size_t aIndex) const;

    std::string getLowerBoundVectorName() const;
    std::string getLowerBoundValueName() const;
    void setLowerBoundValueName(const std::string & aInput);
    void setLowerBoundVectorName(const std::string & aInput);

    std::string getUpperBoundVectorName() const;
    std::string getUpperBoundValueName() const;
    void setUpperBoundValueName(const std::string & aInput);
    void setUpperBoundVectorName(const std::string & aInput);

    std::vector<std::string> getDescentDirectionNames() const;
    std::string getDescentDirectionName(const size_t aIndex) const;
    void addDescentDirectionName(const std::string & aInput);
    void setDescentDirectionNames(const std::vector<std::string> & aInput);

    std::string getOutputStageName() const;
    void setOutputStageName(const std::string & aInput);

    std::string getCacheStageName() const;
    void setCacheStageName(const std::string & aInput);

    std::vector<std::string> getUpdateProblemStageNames() const;
    void setUpdateProblemStageNames(const std::vector<std::string> & aInput);

    const std::string& getObjectiveValueOutputName() const;
    void setObjectiveValueOutputName(const std::string & aInput);
    const std::string& getObjectiveValueStageName() const;
    void setObjectiveValueStageName(const std::string & aInput);

    const std::string& getObjectiveHessianOutputName() const;
    void setObjectiveHessianOutputName(const std::string & aInput);
    const std::string& getObjectiveHessianStageName() const;
    void setObjectiveHessianStageName(const std::string & aInput);

    const std::string& getObjectiveGradientOutputName() const;
    void setObjectiveGradientOutputName(const std::string & aInput);
    const std::string& getObjectiveGradientStageName() const;
    void setObjectiveGradientStageName(const std::string & aInput);

    std::string getInitializationStageName() const;
    void setInitializationStageName(const std::string & aInput);

    const std::string& getControlResetStageName() const;
    void setControlResetStageName(const std::string & aInput);

    std::string getInitialControlDataName() const;
    void setInitialControlDataName(const std::string & aInput);

    const std::vector<StochasticSampleSharedDataNames>& getStochasticSampleSharedDataNames() const;
    void setStochasticSampleSharedDataNames(std::vector<StochasticSampleSharedDataNames> aStochasticSampleSharedDataNames);

    /******************************************************************************//**
     * @brief Return finalization stage name: stage responsible for writing output files
     * @return stage name
    ***********************************************************************************/
    std::string getFinalizationStageName() const;

    /******************************************************************************//**
     * @brief Set finalization stage name: stage responsible for writing output files
     * @param [in] aInput stage name
    ***********************************************************************************/
    void setFinalizationStageName(const std::string & aInput);

    std::string getSetLowerBoundsStageName() const;
    void setSetLowerBoundsStageName(const std::string & aInput);

    std::string getSetUpperBoundsStageName() const;
    void setSetUpperBoundsStageName(const std::string & aInput);

    size_t getNumConstraints() const;

    /******************************************************************************//**
     * @brief Return list of names assign to constraint shared values
     * @return list with the names assign to constraint shared values
    ***********************************************************************************/
    std::vector<std::string> getConstraintValueNames() const;

    /******************************************************************************//**
     * @brief Return name assign to constraint shared value
     * @param [in] aIndex constraint index
     * @return name assign to constraint shared value
    ***********************************************************************************/
    std::string getConstraintValueName(const size_t aIndex) const;

    /******************************************************************************//**
     * @brief Add constraint shared value name to list
     * @param [in] aInput constraint shared value name
    ***********************************************************************************/
    void addConstraintValueName(const std::string & aValueName);

    /******************************************************************************//**
     * @brief Set list of constraint shared value names
     * @param [in] aInput list of constraint shared value names
    ***********************************************************************************/
    void setConstraintValueNames(const std::vector<std::string> & aInput);

    double getConstraintNormalizedTargetValue(const size_t aIndex) const;
    double getConstraintNormalizedTargetValue(const std::string & aValueName) const;
    void addConstraintNormalizedTargetValue(const std::string & aValueName, const double & aInput);
    bool constraintNormalizedTargetValueWasSet(const std::string & aValueName) const;

    double getConstraintAbsoluteTargetValue(const size_t aIndex) const;
    double getConstraintAbsoluteTargetValue(const std::string & aValueName) const;
    void addConstraintAbsoluteTargetValue(const std::string & aValueName, const double & aInput);
    bool constraintAbsoluteTargetValueWasSet(const std::string & aValueName) const;

    double getConstraintReferenceValue(const size_t aIndex) const;
    double getConstraintReferenceValue(const std::string & aValueName) const;
    void addConstraintReferenceValue(const std::string & aValueName, const double & aInput);
    bool constraintReferenceValueWasSet(const std::string & aValueName) const;

    std::string getConstraintReferenceValueName(const size_t aIndex) const;
    std::string getConstraintReferenceValueName(const std::string & aValueName) const;
    void addConstraintReferenceValueName(const std::string & aValueName, const std::string & aReferenceValueName);

    std::string getConstraintHessianName(const size_t aIndex) const;
    std::string getConstraintHessianName(const std::string & aValueName) const;
    void addConstraintHessianName(const std::string & aValueName, const std::string & aHessianName);

    std::string getConstraintGradientName(const size_t aIndex) const;
    std::string getConstraintGradientName(const std::string & aValueName) const;
    void addConstraintGradientName(const std::string & aValueName, const std::string & aGradientName);

    /******************************************************************************//**
     * @brief Return list of names assign to the constraint value stages
     * @return list of names assign to the constraint value stages
    ***********************************************************************************/
    std::vector<std::string> getConstraintValueStageNames() const;

    /******************************************************************************//**
     * @brief Return name assign to constraint value stage
     * @param [in] aIndex constraint index
     * @return name assign to constraint value stage
    ***********************************************************************************/
    std::string getConstraintValueStageName(const size_t aIndex) const;

    /******************************************************************************//**
     * @brief Add constraint value stage name to list
     * @param [in] aInput constraint value stage name
    ***********************************************************************************/
    void addConstraintValueStageName(const std::string & aInput);

    /******************************************************************************//**
     * @brief Set list of constraint value stage names
     * @param [in] aInput list of constraint value stage names
    ***********************************************************************************/
    void setConstraintValueStageNames(const std::vector<std::string> & aInput);

    /******************************************************************************//**
     * @brief Return list of names assign to the constraint gradient stages
     * @return list of names assign to the constraint gradient stages
    ***********************************************************************************/
    std::vector<std::string> getConstraintGradientStageNames() const;

    /******************************************************************************//**
     * @brief Return name assign to constraint gradient stage
     * @param [in] aIndex constraint index
     * @return name assign to constraint gradient stage
    ***********************************************************************************/
    std::string getConstraintGradientStageName(const size_t aIndex) const;

    /******************************************************************************//**
     * @brief Add constraint gradient stage name to list
     * @param [in] aInput constraint gradient stage name
    ***********************************************************************************/
    void addConstraintGradientStageName(const std::string & aInput);

    /******************************************************************************//**
     * @brief Set list of constraint gradient stage names
     * @param [in] aInput list of constraint gradient stage names
    ***********************************************************************************/
    void setConstraintGradientStageNames(const std::vector<std::string> & aInput);

    /******************************************************************************//**
     * @brief Return list of names assign to the constraint Hessian stages
     * @return list of names assign to the constraint Hessian stages
    ***********************************************************************************/
    std::vector<std::string> getConstraintHessianStageNames() const;

    /******************************************************************************//**
     * @brief Return name assign to constraint Hessian stage
     * @param [in] aIndex constraint index
     * @return name assign to constraint Hessian stage
    ***********************************************************************************/
    std::string getConstraintHessianStageName(const size_t aIndex) const;

    /******************************************************************************//**
     * @brief Add constraint Hessian stage name to list
     * @param [in] aInput constraint Hessian stage name
    ***********************************************************************************/
    void addConstraintHessianStageName(const std::string & aInput);

    /******************************************************************************//**
     * @brief Set list of constraint Hessian stage names
     * @param [in] aInput list of constraint Hessian stage names
    ***********************************************************************************/
    void setConstraintHessianStageNames(const std::vector<std::string> & aInput);

    size_t getProblemUpdateFrequency() const;
    void setProblemUpdateFrequency(const size_t aInput);

    bool getResetAlgorithmOnUpdate() const;
    void setResetAlgorithmOnUpdate(const bool aInput);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
      aArchive & boost::serialization::make_nvp("InputData",boost::serialization::base_object<Plato::InputData>(*this));
      aArchive & boost::serialization::make_nvp("MeanNorm",mMeanNorm);
      aArchive & boost::serialization::make_nvp("CheckGradient",mCheckGradient);
      aArchive & boost::serialization::make_nvp("CheckHessian",mCheckHessian);
      aArchive & boost::serialization::make_nvp("UserInitialGuess",mUserInitialGuess);
      aArchive & boost::serialization::make_nvp("OutputControlToFile",mOutputControlToFile);
      aArchive & boost::serialization::make_nvp("OutputDiagnosticsToFile",mOutputDiagnosticsToFile);
      aArchive & boost::serialization::make_nvp("ResetAlgorithmOnUpdate",mResetAlgorithmOnUpdate);

      aArchive & boost::serialization::make_nvp("MaxNumIterations",mMaxNumIterations);
      aArchive & boost::serialization::make_nvp("ProblemUpdateFrequency",mProblemUpdateFrequency);

      aArchive & boost::serialization::make_nvp("ROLCheckGradientSteps",mROLCheckGradientSteps);
      aArchive & boost::serialization::make_nvp("ROLCheckGradientStepSize",mROLCheckGradientStepSize);
      aArchive & boost::serialization::make_nvp("ROLCheckGradientSeed",mROLCheckGradientSeed);

      aArchive & boost::serialization::make_nvp("ROLStochasticDistributionsFile",mROLStochasticDistributionsFile);
      aArchive & boost::serialization::make_nvp("ROLStochasticNumberOfSamples",mROLStochasticNumberOfSamples);
      aArchive & boost::serialization::make_nvp("ROLStochasticSamplerSeed",mROLStochasticSamplerSeed);

      aArchive & boost::serialization::make_nvp("ROLPerturbationScale",mROLPerturbationScale);
      
      aArchive & boost::serialization::make_nvp("StateName",mStateName);
      aArchive & boost::serialization::make_nvp("HessianType",mHessianType);
      aArchive & boost::serialization::make_nvp("InputFileName",mInputFileName);
      aArchive & boost::serialization::make_nvp("CacheStageName",mCacheStageName);
      aArchive & boost::serialization::make_nvp("OutputStageName",mOutputStageName);
      aArchive & boost::serialization::make_nvp("ObjectiveValueOutputName",mObjectiveValueOutputName);
      aArchive & boost::serialization::make_nvp("ObjectiveGradientOutputName",mObjectiveGradientOutputName);
      aArchive & boost::serialization::make_nvp("ObjectiveHessianOutputName",mObjectiveHessianOutputName);
      aArchive & boost::serialization::make_nvp("InitializationStageName",mInitializationStageName);
      aArchive & boost::serialization::make_nvp("ControlResetStageName",mControlResetStageName);
      aArchive & boost::serialization::make_nvp("InitialControlDataName",mInitialControlDataName);
      aArchive & boost::serialization::make_nvp("FinalizationStageName",mFinalizationStageName);
      aArchive & boost::serialization::make_nvp("SetLowerBoundsStageName",mSetLowerBoundsStageName);
      aArchive & boost::serialization::make_nvp("SetUpperBoundsStageName",mSetUpperBoundsStageName);
      aArchive & boost::serialization::make_nvp("LowerBoundValueName",mLowerBoundValueName);
      aArchive & boost::serialization::make_nvp("LowerBoundVectorName",mLowerBoundVectorName);
      aArchive & boost::serialization::make_nvp("UpperBoundValueName",mUpperBoundValueName);
      aArchive & boost::serialization::make_nvp("UpperBoundVectorName",mUpperBoundVectorName);
      aArchive & boost::serialization::make_nvp("ObjectiveValueStageName",mObjectiveValueStageName);
      aArchive & boost::serialization::make_nvp("ObjectiveGradientStageName",mObjectiveGradientStageName);
      aArchive & boost::serialization::make_nvp("ObjectiveHessianStageName",mObjectiveHessianStageName);
      aArchive & boost::serialization::make_nvp("StochasticSampleSharedDataNames",mStochasticSampleSharedDataNames);
      
      aArchive & boost::serialization::make_nvp("InitialGuess",mInitialGuess);
      aArchive & boost::serialization::make_nvp("LowerBoundValues",mLowerBoundValues);
      aArchive & boost::serialization::make_nvp("UpperBoundValues",mUpperBoundValues);

      aArchive & boost::serialization::make_nvp("ConstraintNormalizedTargetValues",mConstraintNormalizedTargetValues);
      aArchive & boost::serialization::make_nvp("ConstraintAbsoluteTargetValues",mConstraintAbsoluteTargetValues);
      aArchive & boost::serialization::make_nvp("ConstraintReferenceValues",mConstraintReferenceValues);

      aArchive & boost::serialization::make_nvp("ControlNames",mControlNames);
      aArchive & boost::serialization::make_nvp("FilteredControlNames",mFilteredControlNames);
      aArchive & boost::serialization::make_nvp("ConstraintValueNames",mConstraintValueNames);
      aArchive & boost::serialization::make_nvp("DescentDirectionNames",mDescentDirectionNames);
      aArchive & boost::serialization::make_nvp("ConstraintValueStageNames",mConstraintValueStageNames);
      aArchive & boost::serialization::make_nvp("ConstraintGradientStageNames",mConstraintGradientStageNames);
      aArchive & boost::serialization::make_nvp("ConstraintHessianStageNames",mConstraintHessianStageNames);
      aArchive & boost::serialization::make_nvp("UpdateProblemStageNames",mUpdateProblemStageNames);

      aArchive & boost::serialization::make_nvp("ConstraintGradientNames",mConstraintGradientNames);
      aArchive & boost::serialization::make_nvp("ConstraintHessianNames",mConstraintHessianNames);
      aArchive & boost::serialization::make_nvp("ConstraintReferenceValueNames",mConstraintReferenceValueNames);
    }

private:
    bool mMeanNorm = false;
    bool mCheckGradient = false;
    bool mCheckHessian = false;
    bool mUserInitialGuess = false;
    bool mOutputControlToFile = false;
    bool mOutputDiagnosticsToFile = true;
    bool mResetAlgorithmOnUpdate = false;

    size_t mMaxNumIterations = 500;
    size_t mProblemUpdateFrequency = 0;

    int mROLCheckGradientSteps = 12;
    int mROLCheckGradientStepSize = 10;
    int mROLCheckGradientSeed = 0;
    std::string mROLStochasticDistributionsFile = "distributions.xml";
    int mROLStochasticNumberOfSamples = 3;
    int mROLStochasticSamplerSeed = 42;

    double mROLPerturbationScale = 1.0;

    std::string mStateName;
    std::string mHessianType = "disabled";
    std::string mInputFileName;
    std::string mCacheStageName;
    std::string mOutputStageName;
    std::string mObjectiveValueOutputName;
    std::string mObjectiveGradientOutputName;
    std::string mObjectiveHessianOutputName;
    std::string mInitializationStageName;
    std::string mControlResetStageName;
    std::string mInitialControlDataName;
    std::string mFinalizationStageName;
    std::string mSetLowerBoundsStageName;
    std::string mSetUpperBoundsStageName;
    std::string mLowerBoundValueName;
    std::string mLowerBoundVectorName;
    std::string mUpperBoundValueName;
    std::string mUpperBoundVectorName;
    std::string mObjectiveValueStageName;
    std::string mObjectiveGradientStageName;
    std::string mObjectiveHessianStageName;
    
    std::vector<StochasticSampleSharedDataNames> mStochasticSampleSharedDataNames;

    std::vector<double> mInitialGuess = {0.5};
    std::vector<double> mLowerBoundValues = {0.0};
    std::vector<double> mUpperBoundValues = {1.0};

    std::map<std::string, double> mConstraintNormalizedTargetValues;
    std::map<std::string, double> mConstraintAbsoluteTargetValues;
    std::map<std::string, double> mConstraintReferenceValues;

    std::vector<std::string> mControlNames;
    std::vector<std::string> mFilteredControlNames;
    std::vector<std::string> mConstraintValueNames;
    std::vector<std::string> mDescentDirectionNames;
    std::vector<std::string> mConstraintValueStageNames;
    std::vector<std::string> mConstraintGradientStageNames;
    std::vector<std::string> mConstraintHessianStageNames;
    std::vector<std::string> mUpdateProblemStageNames;

    std::map<std::string, std::string> mConstraintGradientNames;
    std::map<std::string, std::string> mConstraintHessianNames;
    std::map<std::string, std::string> mConstraintReferenceValueNames;
};

} // namespace Plato

#endif /* PLATO_OPTIMIZERENGINESTAGEDATA_HPP_ */
