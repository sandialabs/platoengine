/*
 * Plato_OptimizerInterface.hpp
 *
 *  Created on: Oct 30, 2017
 */

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

#ifndef PLATO_OPTIMIZERINTERFACE_HPP_
#define PLATO_OPTIMIZERINTERFACE_HPP_

#include "Plato_DriverInterface.hpp"
#include "Plato_OptimizerUtilities.hpp"

#include <mpi.h>

namespace Plato
{

struct optimizer
{
    enum algorithm_t
    {
        SO_PARAMETER_STUDIES = 13,
        ROL_LINEAR_CONSTRAINT = 14,
        ROL_BOUND_CONSTRAINED = 15,
        ROL_AUGMENTED_LAGRANGIAN = 16
    }; // enum optimizer_t
};
// struct optimizer

/******************************************************************************//**
 * @brief Abstract interface to optimization algorithm
**********************************************************************************/
template<typename ScalarType, typename OrdinalType = size_t>
class OptimizerInterface : public DriverInterface<ScalarType, OrdinalType>
{
public:
    OptimizerInterface(Plato::Interface* aInterface, const MPI_Comm & aComm):
        mComm(aComm), mInterface(aInterface),
        mInputData(Plato::OptimizerEngineStageData()) { }

    virtual ~OptimizerInterface() = default;

    /******************************************************************************//**
     * @brief Return true if the last driver
    **********************************************************************************/
    bool lastDriver() const override { return lastOptimizer(); }

    /******************************************************************************//**
     * @brief Return the driver type
     * \return driver type
    **********************************************************************************/
    Plato::driver_t driver() const override
    {
        return (Plato::driver_t::PLATO_OPTIMIZER_DRIVER);
    }

    /******************************************************************************//**
     * @brief Return the algorithm type
     * \return algorithm type
    **********************************************************************************/
    virtual optimizer::algorithm_t algorithm() const = 0;

    /******************************************************************************//**
     * @brief Get the optimizer node based on the index.
    **********************************************************************************/
    Plato::InputData getOptimizerNode(Plato::Interface* aInterface)
    {
        return Plato::getOptimizerNode(aInterface, mOptimizerIndex);
    }

    /******************************************************************************//**
     * @brief Set the optimizer name - Optional.
    **********************************************************************************/
    void setOptimizerName( std::string val ) { mOptimizerName = val; }

    /******************************************************************************//**
     * @brief Get the optimizer name.
    **********************************************************************************/
    std::string getOptimizerName() const { return mOptimizerName; }

    /******************************************************************************//**
     * @brief Set the index of the optimizer.

     // The optimizer block index is a vector of indices. The size of
     // the vector less 1 denotes the number of nesting levels. Each
     // index is the serial index of the optimizer block.

     // A basic run with one optimizer block would have a vector of
     // {0}. A simple nested run with one nested optimizer block would
     // have a vector of {0} for the outer optimizer block and {0,0}
     // for the inner optimizer block.

     // A more complicated vector for the inner most optimizer block
     // would be {2,1,0} which denotes the outer most third serial
     // optimizer block {2}, of which its second serial inner loop
     // optimizer block is wanted {2,1}, of which its first inner loop
     // optimizer block is wanted {2,1,0},

    **********************************************************************************/
    void setOptimizerIndex( std::vector<size_t> val ) { mOptimizerIndex = val; }

    /******************************************************************************//**
     * @brief Get the index of the optimizer.
    **********************************************************************************/
    std::vector<size_t> getOptimizerIndex() const { return mOptimizerIndex; }

    /******************************************************************************//**
     * @brief Set the inner loop boolean.
    **********************************************************************************/
    void setHasInnerLoop( bool val ) { mHasInnerLoop = val; }

    /******************************************************************************//**
     * @brief Get the inner loop boolean.
    **********************************************************************************/
    bool getHasInnerLoop() const { return mHasInnerLoop; }

    /******************************************************************************//**
     * @brief True if this optimizer is the last top level serial
     * optimizer to be executed.
    **********************************************************************************/
    void lastOptimizer( bool val ) { mLastOptimizer = val; }

    /******************************************************************************//**
     * @brief True if this optimizer is the last top level serial
     * optimizer to be executed.
    **********************************************************************************/
    bool lastOptimizer() const { return mLastOptimizer; }

    void initialize() override
    {
        Plato::initialize<ScalarType, OrdinalType>(this->mInterface, this->mInputData, this->mOptimizerIndex);
    }
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
      aArchive & boost::serialization::make_nvp("OptimizerName",mOptimizerName);
      aArchive & boost::serialization::make_nvp("OptimizerIndex",mOptimizerIndex);
      aArchive & boost::serialization::make_nvp("LastOptimizer",mLastOptimizer);
      aArchive & boost::serialization::make_nvp("HasInnerLoop",mHasInnerLoop);
      //aArchive & boost::serialization::make_nvp("Interface",*mInterface);<<<this is already serialized 
      aArchive & boost::serialization::make_nvp("InputData",mInputData);
      aArchive & boost::serialization::make_nvp("StageDataMng",mStageDataMng);
    }

protected:
    /******************************************************************************//**
     * @brief String containing the optimizer name - Optional
    **********************************************************************************/
    std::string mOptimizerName{""};

    /******************************************************************************//**
     * @brief Vector of indices decribing this optimizer's location
     * in the input - used when there are multiple optimizers. See
     * Plato_OptimizerUtilities.hpp and the function getOptimizerNode().
    **********************************************************************************/
    std::vector<size_t> mOptimizerIndex;

    /******************************************************************************//**
     * @brief Boolean indicating if this optimizer is the last top
     * level serial optimizer to be executed and can issue a
     * finialize/terminate operation.
    **********************************************************************************/
    bool mLastOptimizer{false};

    /******************************************************************************//**
     * @brief Boolean indicating an inner optimizer loop is present
    **********************************************************************************/
    bool mHasInnerLoop{false};

    MPI_Comm mComm;
    Plato::Interface* mInterface;
    Plato::OptimizerEngineStageData mInputData;
    Plato::StageInputDataMng mStageDataMng;
};
// class OptimizerInterface

} // namespace Plato

#endif /* PLATO_OPTIMIZERINTERFACE_HPP_ */
