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
 * Plato_OptimizerEngineStageData.cpp
 *
 *  Created on: Oct 27, 2017
 */

#include <cassert>
#include <iostream>

#include "Plato_OptimizerEngineStageData.hpp"

namespace Plato
{

/******************************************************************************/
std::vector<double> OptimizerEngineStageData::getLowerBoundValues() const
/******************************************************************************/
{
    return (mLowerBoundValues);
}

/******************************************************************************/
std::vector<double> OptimizerEngineStageData::getUpperBoundValues() const
/******************************************************************************/
{
    return (mUpperBoundValues);
}

/******************************************************************************/
void OptimizerEngineStageData::setLowerBoundValues(const std::vector<double> & aInput)
/******************************************************************************/
{
    mLowerBoundValues = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setUpperBoundValues(const std::vector<double> & aInput)
/******************************************************************************/
{
    mUpperBoundValues = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setOutputControlToFile(const bool aInput)
/******************************************************************************/
{
    mOutputControlToFile = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setOutputDiagnosticsToFile(const bool aInput)
{
    mOutputDiagnosticsToFile = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setMeanNorm(const bool aInput)
{
    mMeanNorm = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setHessianType(const std::string & aInput)
{
    mHessianType = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setCheckGradient(const bool aInput)
{
    mCheckGradient = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setCheckHessian(const bool aInput)
/******************************************************************************/
{
    mCheckHessian = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setUserInitialGuess(const bool aInput)
/******************************************************************************/
{
    mUserInitialGuess = aInput;
}

/******************************************************************************/
bool OptimizerEngineStageData::getOutputControlToFile() const
/******************************************************************************/
{
    return (mOutputControlToFile);
}

/******************************************************************************/
bool OptimizerEngineStageData::getOutputDiagnosticsToFile() const
{
    return (mOutputDiagnosticsToFile);
}

/******************************************************************************/
bool OptimizerEngineStageData::getMeanNorm() const
{
    return mMeanNorm;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getHessianType() const
{
    return mHessianType;
}

/******************************************************************************/
bool OptimizerEngineStageData::getCheckGradient() const
{
    return mCheckGradient;
}

/******************************************************************************/
bool OptimizerEngineStageData::getCheckHessian() const
{
    return mCheckHessian;
}

/******************************************************************************/
bool OptimizerEngineStageData::getUserInitialGuess() const
/******************************************************************************/
{
    return (mUserInitialGuess);
}

/******************************************************************************/
size_t OptimizerEngineStageData::getMaxNumIterations() const
/******************************************************************************/
{
    return (mMaxNumIterations);
}

/******************************************************************************/
void OptimizerEngineStageData::setMaxNumIterations(const size_t aInput)
/******************************************************************************/
{
    mMaxNumIterations = aInput;
}

/******************************************************************************/
std::vector<double> OptimizerEngineStageData::getInitialGuess() const
/******************************************************************************/
{
    return (mInitialGuess);
}

/******************************************************************************/
void OptimizerEngineStageData::setInitialGuess(const std::vector<double> & aInput)
/******************************************************************************/
{
    mInitialGuess = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setInitialGuess(const std::vector<std::string> & aInput)
/******************************************************************************/
{
    mInitialGuess.resize(aInput.size());
    for(unsigned int i=0; i<aInput.size(); i++)
    {
       mInitialGuess[i] = std::strtold(aInput[i].c_str(), nullptr);
    }
}

double OptimizerEngineStageData::getROLPerturbationScale() const
{
    return mROLPerturbationScale;
}
void OptimizerEngineStageData::setROLPerturbationScale(const double aInput)
{
    mROLPerturbationScale = aInput;
}
int OptimizerEngineStageData::getROLCheckGradientSteps() const
{
    return mROLCheckGradientSteps;
}
void OptimizerEngineStageData::setROLCheckGradientSteps(const int aInput)
{
    mROLCheckGradientSteps = aInput;
}
int OptimizerEngineStageData::getROLCheckGradientStepSize() const
{
    return mROLCheckGradientStepSize;
}
void OptimizerEngineStageData::setROLCheckGradientStepSize(const int aInput)
{
    mROLCheckGradientStepSize = aInput;
}
int OptimizerEngineStageData::getROLCheckGradientSeed() const
{
    return mROLCheckGradientSeed;
}
void OptimizerEngineStageData::setROLCheckGradientSeed(const int aInput)
{
    mROLCheckGradientSeed = aInput;
}

const std::string& OptimizerEngineStageData::getROLStochasticDistributionsFile() const
{
    return mROLStochasticDistributionsFile;
}

void OptimizerEngineStageData::setROLStochasticDistributionsFile(std::string aInput)
{
    mROLStochasticDistributionsFile = std::move(aInput);
}

int OptimizerEngineStageData::getROLStochasticNumberOfSamples() const
{
    return mROLStochasticNumberOfSamples;
}

void OptimizerEngineStageData::setROLStochasticNumberOfSamples(const int aInput)
{
    mROLStochasticNumberOfSamples = aInput;
}

int OptimizerEngineStageData::getROLStochasticSamplerSeed() const
{
    return mROLStochasticSamplerSeed;
}

void OptimizerEngineStageData::setROLStochasticSamplerSeed(const int aInput)
{
    mROLStochasticSamplerSeed = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getStateName() const
{
    assert(mStateName.empty() == false);
    return (mStateName);
}

/******************************************************************************/
void OptimizerEngineStageData::setStateNames(const std::string & aInput)
{
    assert(aInput.empty() == false);
    mStateName = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getInputFileName() const
{
    return (mInputFileName);
}

/******************************************************************************/
void OptimizerEngineStageData::setInputFileName(const std::string & aInput)
/******************************************************************************/
{
    mInputFileName = aInput;
}

/******************************************************************************/
size_t OptimizerEngineStageData::getNumControlVectors() const
/******************************************************************************/
{
    assert(mControlNames.empty() == false);
    return (mControlNames.size());
}

/******************************************************************************/
std::string OptimizerEngineStageData::getLowerBoundVectorName() const
/******************************************************************************/
{
    return mLowerBoundVectorName;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getLowerBoundValueName() const
/******************************************************************************/
{
    return mLowerBoundValueName;
}

/******************************************************************************/
void OptimizerEngineStageData::setLowerBoundValueName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mLowerBoundValueName = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setLowerBoundVectorName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mLowerBoundVectorName = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getUpperBoundVectorName() const
/******************************************************************************/
{
    return mUpperBoundVectorName;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getUpperBoundValueName() const
/******************************************************************************/
{
    return mUpperBoundValueName;
}

/******************************************************************************/
void OptimizerEngineStageData::setUpperBoundValueName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mUpperBoundValueName = aInput;
}

/******************************************************************************/
void OptimizerEngineStageData::setUpperBoundVectorName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mUpperBoundVectorName = aInput;
}

/******************************************************************************/
std::vector<std::string> OptimizerEngineStageData::getControlNames() const
/******************************************************************************/
{
    assert(mControlNames.empty() == false);
    return (mControlNames);
}

/******************************************************************************/
std::string OptimizerEngineStageData::getControlName(const size_t aInput) const
/******************************************************************************/
{
    assert(mControlNames.empty() == false);
    assert(aInput < mControlNames.size());
    return (mControlNames[aInput]);
}

/******************************************************************************/
void OptimizerEngineStageData::addControlName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mControlNames.push_back(aInput);
}

/******************************************************************************/
void OptimizerEngineStageData::setControlNames(const std::vector<std::string> & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mControlNames = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getFilteredControlName(const size_t aInput) const
/******************************************************************************/
{
    assert(mFilteredControlNames.empty() == false);
    assert(aInput < mFilteredControlNames.size());
    return (mFilteredControlNames[aInput]);
}

/******************************************************************************/
void OptimizerEngineStageData::addFilteredControlName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mFilteredControlNames.push_back(aInput);
}

std::vector<std::string> OptimizerEngineStageData::getDescentDirectionNames() const
{
    assert(mDescentDirectionNames.empty() == false);
    return (mDescentDirectionNames);
}

/******************************************************************************/
std::string OptimizerEngineStageData::getDescentDirectionName(const size_t aInput) const
/******************************************************************************/
{
    assert(mDescentDirectionNames.empty() == false);
    assert(aInput < mDescentDirectionNames.size());
    return (mDescentDirectionNames[aInput]);
}

/******************************************************************************/
void OptimizerEngineStageData::addDescentDirectionName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mDescentDirectionNames.push_back(aInput);
}

/******************************************************************************/
void OptimizerEngineStageData::setDescentDirectionNames(const std::vector<std::string> & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mDescentDirectionNames = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getOutputStageName() const
/******************************************************************************/
{
    return (mOutputStageName);
}

/******************************************************************************/
void OptimizerEngineStageData::setOutputStageName(const std::string & aInput)
/******************************************************************************/
{
    mOutputStageName.clear();
    mOutputStageName = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getCacheStageName() const
/******************************************************************************/
{
    return (mCacheStageName);
}
/******************************************************************************/
void OptimizerEngineStageData::setCacheStageName(const std::string & aInput)
/******************************************************************************/
{
    mCacheStageName.clear();
    mCacheStageName = aInput;
}

/******************************************************************************/
std::vector< std::string > OptimizerEngineStageData::getUpdateProblemStageNames() const
/******************************************************************************/
{
    return (mUpdateProblemStageNames);
}
/******************************************************************************/
void OptimizerEngineStageData::setUpdateProblemStageNames(const std::vector< std::string > & aInput)
/******************************************************************************/
{
    mUpdateProblemStageNames = aInput;
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getObjectiveValueOutputName() const
/******************************************************************************/
{
    assert(mObjectiveValueOutputName.empty() == false);
    return mObjectiveValueOutputName;
}

/******************************************************************************/
void OptimizerEngineStageData::setObjectiveValueOutputName(const std::string & aInput)
/******************************************************************************/
{
    mObjectiveValueOutputName.clear();
    mObjectiveValueOutputName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getObjectiveValueStageName() const
/******************************************************************************/
{
    return mObjectiveValueStageName;
}

/******************************************************************************/
void OptimizerEngineStageData::setObjectiveValueStageName(const std::string & aInput)
/******************************************************************************/
{
    mObjectiveValueStageName.clear();
    mObjectiveValueStageName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getObjectiveHessianOutputName() const
/******************************************************************************/
{
    return mObjectiveHessianOutputName;
}

/******************************************************************************/
void OptimizerEngineStageData::setObjectiveHessianOutputName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mObjectiveHessianOutputName.clear();
    mObjectiveHessianOutputName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getObjectiveHessianStageName() const
/******************************************************************************/
{
    return mObjectiveHessianStageName;
}

/******************************************************************************/
void OptimizerEngineStageData::setObjectiveHessianStageName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mObjectiveHessianStageName.clear();
    mObjectiveHessianStageName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getObjectiveGradientOutputName() const
/******************************************************************************/
{
    assert(mObjectiveGradientOutputName.empty() == false);
    return mObjectiveGradientOutputName;
}

/******************************************************************************/
void OptimizerEngineStageData::setObjectiveGradientOutputName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mObjectiveGradientOutputName.clear();
    mObjectiveGradientOutputName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getObjectiveGradientStageName() const
/******************************************************************************/
{
    return mObjectiveGradientStageName;
}

/******************************************************************************/
void OptimizerEngineStageData::setObjectiveGradientStageName(const std::string & aInput)
/******************************************************************************/
{
    mObjectiveGradientStageName.clear();
    mObjectiveGradientStageName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
std::string OptimizerEngineStageData::getInitializationStageName() const
/******************************************************************************/
{
    return (mInitializationStageName);
}

/******************************************************************************/
void OptimizerEngineStageData::setInitializationStageName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mInitializationStageName.clear();
    mInitializationStageName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
const std::string& OptimizerEngineStageData::getControlResetStageName() const
/******************************************************************************/
{
    return mControlResetStageName;
}

/******************************************************************************//**
 * @pre [aInput] must not be empty
 ***********************************************************************************/
void OptimizerEngineStageData::setControlResetStageName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mControlResetStageName = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getInitialControlDataName() const
/******************************************************************************/
{
    if(mInitialControlDataName.empty() == true)
    {
        return this->getControlName(0);
    }
    else
    {
        return (mInitialControlDataName);
    }
}

/******************************************************************************/
void OptimizerEngineStageData::setInitialControlDataName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mInitialControlDataName.clear();
    mInitialControlDataName.assign(aInput.begin(), aInput.end());
}

const std::vector<StochasticSampleSharedDataNames>& OptimizerEngineStageData::getStochasticSampleSharedDataNames() const
{
    return mStochasticSampleSharedDataNames;
}

void OptimizerEngineStageData::setStochasticSampleSharedDataNames(
    std::vector<StochasticSampleSharedDataNames> aStochasticSampleSharedDataNames)
{
    mStochasticSampleSharedDataNames = std::move(aStochasticSampleSharedDataNames);
}

/******************************************************************************/
std::string OptimizerEngineStageData::getFinalizationStageName() const
/******************************************************************************/
{
    return (mFinalizationStageName);
}

/******************************************************************************/
void OptimizerEngineStageData::setFinalizationStageName(const std::string & aInput)
/******************************************************************************/
{
    mFinalizationStageName.clear();
    mFinalizationStageName = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getSetLowerBoundsStageName() const
/******************************************************************************/
{
    return (mSetLowerBoundsStageName);
}

/******************************************************************************/
void OptimizerEngineStageData::setSetLowerBoundsStageName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mSetLowerBoundsStageName.clear();
    mSetLowerBoundsStageName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
std::string OptimizerEngineStageData::getSetUpperBoundsStageName() const
/******************************************************************************/
{
    return (mSetUpperBoundsStageName);
}

/******************************************************************************/
void OptimizerEngineStageData::setSetUpperBoundsStageName(const std::string & aInput)
/******************************************************************************/
{
    assert(aInput.empty() == false);
    mSetUpperBoundsStageName.clear();
    mSetUpperBoundsStageName.assign(aInput.begin(), aInput.end());
}

/******************************************************************************/
size_t OptimizerEngineStageData::getNumConstraints() const
/******************************************************************************/
{
    return (mConstraintValueNames.size());
}

/******************************************************************************/
double OptimizerEngineStageData::getConstraintNormalizedTargetValue(const size_t aIndex) const
/******************************************************************************/
{
    assert(mConstraintNormalizedTargetValues.empty() == false);
    assert(aIndex < static_cast<size_t>(mConstraintNormalizedTargetValues.size()));
    const std::string & tValueName = mConstraintValueNames[aIndex];
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintNormalizedTargetValues.find(tValueName);
    return (tIterator->second);
}

/******************************************************************************/
double OptimizerEngineStageData::getConstraintNormalizedTargetValue(const std::string & aValueName) const
/******************************************************************************/
{
    assert(mConstraintNormalizedTargetValues.empty() == false);
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintNormalizedTargetValues.find(aValueName);
    assert(tIterator != mConstraintNormalizedTargetValues.end());
    return (tIterator->second);
}

/******************************************************************************/
void OptimizerEngineStageData::addConstraintNormalizedTargetValue(const std::string & aValueName, const double & aInput)
/******************************************************************************/
{
    std::map<std::string, double>::iterator tIterator;
    tIterator = mConstraintNormalizedTargetValues.find(aValueName);
    if(tIterator != mConstraintNormalizedTargetValues.end())
    {
        tIterator->second = aInput;
    }
    else
    {
        mConstraintNormalizedTargetValues[aValueName] = aInput;
    }
}

/******************************************************************************/
bool OptimizerEngineStageData::constraintNormalizedTargetValueWasSet(const std::string & aValueName) const
/******************************************************************************/
{
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintNormalizedTargetValues.find(aValueName);
    return (tIterator != mConstraintNormalizedTargetValues.end());
}

/******************************************************************************/
double OptimizerEngineStageData::getConstraintAbsoluteTargetValue(const size_t aIndex) const
/******************************************************************************/
{
    assert(aIndex < static_cast<size_t>(mConstraintAbsoluteTargetValues.size()));
    const std::string & tValueName = mConstraintValueNames[aIndex];
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintAbsoluteTargetValues.find(tValueName);
    return (tIterator->second);
}

/******************************************************************************/
double OptimizerEngineStageData::getConstraintAbsoluteTargetValue(const std::string & aValueName) const
/******************************************************************************/
{
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintAbsoluteTargetValues.find(aValueName);
    if(tIterator == mConstraintAbsoluteTargetValues.end())
    {
        std::cout << "\n\nERROR: Trying to get a constraint's absolute value when it hasn't been set.\n\n";
        return 0.0;
    }
    else
        return (tIterator->second);
}

/******************************************************************************/
void OptimizerEngineStageData::addConstraintAbsoluteTargetValue(const std::string & aValueName, const double & aInput)
/******************************************************************************/
{
    std::map<std::string, double>::iterator tIterator;
    tIterator = mConstraintAbsoluteTargetValues.find(aValueName);
    if(tIterator != mConstraintAbsoluteTargetValues.end())
    {
        tIterator->second = aInput;
    }
    else
    {
        mConstraintAbsoluteTargetValues[aValueName] = aInput;
    }
}

/******************************************************************************/
bool OptimizerEngineStageData::constraintAbsoluteTargetValueWasSet(const std::string & aValueName) const
/******************************************************************************/
{
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintAbsoluteTargetValues.find(aValueName);
    return (tIterator != mConstraintAbsoluteTargetValues.end());
}

/******************************************************************************/
double OptimizerEngineStageData::getConstraintReferenceValue(const size_t aIndex) const
/******************************************************************************/
{
    assert(mConstraintReferenceValues.empty() == false);
    assert(aIndex < static_cast<size_t>(mConstraintReferenceValues.size()));
    const std::string & tValueName = mConstraintValueNames[aIndex];
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintReferenceValues.find(tValueName);
    return (tIterator->second);
}

/******************************************************************************/
double OptimizerEngineStageData::getConstraintReferenceValue(const std::string & aValueName) const
/******************************************************************************/
{
    assert(mConstraintReferenceValues.empty() == false);
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintReferenceValues.find(aValueName);
    assert(tIterator != mConstraintReferenceValues.end());
    return (tIterator->second);
}

/******************************************************************************/
void OptimizerEngineStageData::addConstraintReferenceValue(const std::string & aValueName, const double & aInput)
/******************************************************************************/
{
    std::map<std::string, double>::iterator tIterator;
    tIterator = mConstraintReferenceValues.find(aValueName);
    if(tIterator != mConstraintReferenceValues.end())
    {
        tIterator->second = aInput;
    }
    else
    {
        mConstraintReferenceValues[aValueName] = aInput;
    }
}

/******************************************************************************/
bool OptimizerEngineStageData::constraintReferenceValueWasSet(const std::string & aValueName) const
/******************************************************************************/
{
    std::map<std::string, double>::const_iterator tIterator;
    tIterator = mConstraintReferenceValues.find(aValueName);
    return (tIterator != mConstraintReferenceValues.end());
}

/*************************************************************************************/
std::vector<std::string> OptimizerEngineStageData::getConstraintValueNames() const
{
    assert(mConstraintValueNames.empty() == false);
    return (mConstraintValueNames);
}

/*************************************************************************************/
std::string OptimizerEngineStageData::getConstraintValueName(const size_t aIndex) const
{
    assert(mConstraintValueNames.empty() == false);
    assert(aIndex < mConstraintValueNames.size());
    return (mConstraintValueNames[aIndex]);
}

/*************************************************************************************/
void OptimizerEngineStageData::addConstraintValueName(const std::string & aInput)
{
    assert(aInput.empty() == false);
    mConstraintValueNames.push_back(aInput);
}

/*************************************************************************************/
void OptimizerEngineStageData::setConstraintValueNames(const std::vector<std::string> & aInput)
{
    assert(aInput.empty() == false);
    mConstraintValueNames = aInput;
}

/*************************************************************************************/
std::vector<std::string> OptimizerEngineStageData::getConstraintValueStageNames() const
{
    assert(mConstraintValueStageNames.empty() == false);
    return (mConstraintValueStageNames);
}

/*************************************************************************************/
std::string OptimizerEngineStageData::getConstraintValueStageName(const size_t aIndex) const
{
    assert(mConstraintValueStageNames.empty() == false);
    assert(aIndex < mConstraintValueStageNames.size());
    return (mConstraintValueStageNames[aIndex]);
}

/*************************************************************************************/
void OptimizerEngineStageData::addConstraintValueStageName(const std::string & aInput)
{
    assert(aInput.empty() == false);
    mConstraintValueStageNames.push_back(aInput);
}

/*************************************************************************************/
void OptimizerEngineStageData::setConstraintValueStageNames(const std::vector<std::string> & aInput)
{
    assert(aInput.empty() == false);
    mConstraintValueStageNames = aInput;
}

/******************************************************************************/
std::string OptimizerEngineStageData::getConstraintReferenceValueName(const size_t aIndex) const
/******************************************************************************/
{
    if(mConstraintReferenceValueNames.empty() == true)
        return "";
    assert(aIndex < static_cast<size_t>(mConstraintReferenceValueNames.size()));
    const std::string & tValueName = mConstraintValueNames[aIndex];
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mConstraintReferenceValueNames.find(tValueName);
    if(tIterator == mConstraintReferenceValueNames.end())
        return "";
    return (tIterator->second);
}

/******************************************************************************/
std::string OptimizerEngineStageData::getConstraintReferenceValueName(const std::string & aValueName) const
/******************************************************************************/
{
    assert(mConstraintReferenceValueNames.empty() == false);
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mConstraintReferenceValueNames.find(aValueName);
    assert(tIterator != mConstraintReferenceValueNames.end());
    return (tIterator->second);
}

/******************************************************************************/
void OptimizerEngineStageData::addConstraintReferenceValueName(const std::string & aValueName,
                                                               const std::string & aReferenceValueName)
/******************************************************************************/
{
    std::map<std::string, std::string>::iterator tIterator;
    tIterator = mConstraintReferenceValueNames.find(aValueName);
    if(tIterator != mConstraintReferenceValueNames.end())
    {
        tIterator->second = aReferenceValueName;
    }
    else
    {
        mConstraintReferenceValueNames[aValueName] = aReferenceValueName;
    }
}

/******************************************************************************/
std::string OptimizerEngineStageData::getConstraintHessianName(const size_t aIndex) const
/******************************************************************************/
{
    assert(mConstraintHessianNames.empty() == false);
    assert(aIndex < static_cast<size_t>(mConstraintHessianNames.size()));
    const std::string & tValueName = mConstraintValueNames[aIndex];
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mConstraintHessianNames.find(tValueName);
    return (tIterator->second);
}

/******************************************************************************/
std::string OptimizerEngineStageData::getConstraintHessianName(const std::string & aValueName) const
/******************************************************************************/
{
    assert(mConstraintHessianNames.empty() == false);
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mConstraintHessianNames.find(aValueName);
    assert(tIterator != mConstraintHessianNames.end());
    return (tIterator->second);
}

/******************************************************************************/
void OptimizerEngineStageData::addConstraintHessianName(const std::string & aValueName, const std::string & aHessianName)
/******************************************************************************/
{
    std::map<std::string, std::string>::iterator tIterator;
    tIterator = mConstraintHessianNames.find(aValueName);
    if(tIterator != mConstraintHessianNames.end())
    {
        tIterator->second = aHessianName;
    }
    else
    {
        mConstraintHessianNames[aValueName] = aHessianName;
    }
}

/******************************************************************************/
std::string OptimizerEngineStageData::getConstraintGradientName(const size_t aIndex) const
/******************************************************************************/
{
    assert(mConstraintGradientNames.empty() == false);
    assert(aIndex < static_cast<size_t>(mConstraintGradientNames.size()));
    const std::string & tValueName = mConstraintValueNames[aIndex];
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mConstraintGradientNames.find(tValueName);
    return (tIterator->second);
}

/******************************************************************************/
std::string OptimizerEngineStageData::getConstraintGradientName(const std::string & aValueName) const
/******************************************************************************/
{
    assert(mConstraintGradientNames.empty() == false);
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mConstraintGradientNames.find(aValueName);
    assert(tIterator != mConstraintGradientNames.end());
    return (tIterator->second);
}

/******************************************************************************/
void OptimizerEngineStageData::addConstraintGradientName(const std::string & aValueName, const std::string & aGradientName)
/******************************************************************************/
{
    std::map<std::string, std::string>::iterator tIterator;
    tIterator = mConstraintGradientNames.find(aValueName);
    if(tIterator != mConstraintGradientNames.end())
    {
        tIterator->second = aGradientName;
    }
    else
    {
        mConstraintGradientNames[aValueName] = aGradientName;
    }
}

/*************************************************************************************/
std::vector<std::string> OptimizerEngineStageData::getConstraintHessianStageNames() const
{
    assert(mConstraintHessianStageNames.empty() == false);
    return (mConstraintHessianStageNames);
}

/*************************************************************************************/
std::string OptimizerEngineStageData::getConstraintHessianStageName(const size_t aIndex) const
{
    assert(mConstraintHessianStageNames.empty() == false);
    assert(aIndex < mConstraintHessianStageNames.size());
    return (mConstraintHessianStageNames[aIndex]);
}

/*************************************************************************************/
void OptimizerEngineStageData::addConstraintHessianStageName(const std::string & aInput)
{
    assert(aInput.empty() == false);
    mConstraintHessianStageNames.push_back(aInput);
}

/*************************************************************************************/
void OptimizerEngineStageData::setConstraintHessianStageNames(const std::vector<std::string> & aInput)
{
    assert(aInput.empty() == false);
    mConstraintHessianStageNames = aInput;
}

/*************************************************************************************/
std::vector<std::string> OptimizerEngineStageData::getConstraintGradientStageNames() const
{
    assert(mConstraintGradientStageNames.empty() == false);
    return (mConstraintGradientStageNames);
}

/*************************************************************************************/
std::string OptimizerEngineStageData::getConstraintGradientStageName(const size_t aIndex) const
{
    assert(mConstraintGradientStageNames.empty() == false);
    assert(aIndex < mConstraintGradientStageNames.size());
    return (mConstraintGradientStageNames[aIndex]);
}

/*************************************************************************************/
void OptimizerEngineStageData::addConstraintGradientStageName(const std::string & aInput)
{
    assert(aInput.empty() == false);
    mConstraintGradientStageNames.push_back(aInput);
}

/*************************************************************************************/
void OptimizerEngineStageData::setConstraintGradientStageNames(const std::vector<std::string> & aInput)
{
    assert(aInput.empty() == false);
    mConstraintGradientStageNames = aInput;
}

size_t OptimizerEngineStageData::getProblemUpdateFrequency() const
{
    return mProblemUpdateFrequency;
}

void OptimizerEngineStageData::setProblemUpdateFrequency(const size_t aInput)
{
    mProblemUpdateFrequency = aInput;
}

bool OptimizerEngineStageData::getResetAlgorithmOnUpdate() const
{
    return mResetAlgorithmOnUpdate;
}

void OptimizerEngineStageData::setResetAlgorithmOnUpdate(const bool aInput)
{
    mResetAlgorithmOnUpdate = aInput;
}

} //namespace Plato
