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
 * Plato_SharedData.hpp
 *
 *  Created on: April 23, 2017
 *
 */

#ifndef SRC_SHAREDDATA_HPP_
#define SRC_SHAREDDATA_HPP_

#include <string>
#include <vector>
#include <cassert>

namespace Plato
{
struct CommunicationData;

//!  Inter-program shared data
/*!
 */


/******************************************************************************//**
 * \brief Allowable data format through the Plato Engine architecture.  Only these
 * data formats are supported through the Multiple-Program, Multiple-Data (MPMD)
 * architecture.
**********************************************************************************/
struct data
{
    enum layout_t
    {
        SCALAR = 0,
        SCALAR_FIELD = 1,
        VECTOR_FIELD = 2,
        TENSOR_FIELD = 3,
        ELEMENT_FIELD = 4,
        SCALAR_PARAMETER = 5,
        UNDEFINED = 6
    };
};

/******************************************************************************//**
 * \brief Shared Data abstract class
**********************************************************************************/
class SharedData
{
public:
    virtual ~SharedData() = default;

    /******************************************************************************//**
     * \brief Return the size of the SharedData container
     * \return container size
    **********************************************************************************/
    virtual int size() const = 0;

    /******************************************************************************//**
     * \brief Return the SharedData name
     * \return data name
    **********************************************************************************/
    virtual std::string myName() const = 0;

    /******************************************************************************//**
     * \brief Return description on the SharedData
     * \return description
    **********************************************************************************/
    virtual std::string myContext() const { return std::string(); }

    /******************************************************************************//**
     * \brief Set the context of the shared data, typically the name of the operation
     *  for a parameter.
    **********************************************************************************/
    virtual void setMyContext(std::string /*aContext*/) {}

    /******************************************************************************//**
     * \brief Return data layout/format
     * \return data layout/format
    **********************************************************************************/
    virtual Plato::data::layout_t myLayout() const = 0;

    /******************************************************************************//**
     * \brief Transmit data from owner application to user application
    **********************************************************************************/
    virtual void transmitData() = 0;

    /******************************************************************************//**
     * \brief Set SharedData container values
     * \param [in] aData standard vector
    **********************************************************************************/
    virtual void setData(const std::vector<double> & aData) = 0;

    /******************************************************************************//**
     * \brief Get SharedData container values
     * \param [in] aData standard vector
    **********************************************************************************/
    virtual void getData(std::vector<double> & aData) const = 0;

    /******************************************************************************//**
     * \brief Return whether this shared data is dynamic
    **********************************************************************************/
    virtual bool isDynamic() const = 0;

    template<class Archive>
    void serialize(Archive & /*aArchive*/, const unsigned int /*version*/){}

    virtual void initializeMPI(const Plato::CommunicationData& /*aCommData*/){}
};
// class SharedData

}
// End namespace Plato

#include <boost/serialization/assume_abstract.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Plato::SharedData)

#endif
