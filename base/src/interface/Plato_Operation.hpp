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
 * Plato_Operation.hpp
 *
 *  Created on: March 30, 2018
 *
 */

#ifndef SRC_OPERATION_HPP_
#define SRC_OPERATION_HPP_

#include "Plato_SerializationHeaders.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_SharedField.hpp"
#include "Plato_Performer.hpp"

#include <boost/serialization/shared_ptr.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Plato
{

class Performer;
class OperationInputDataMng;

//! Performer with input and output shared fields.
/*!
 */
class Operation
{
   
public:
    virtual ~Operation() = default;

    virtual void update(const ::Plato::OperationInputDataMng & aOperationDataMng,
                        const std::shared_ptr<::Plato::Performer> aPerformer,
                        const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData) = 0;

    void sendInput();
    void sendOutput();
    void sendParameters();
    void compute();

    void importData(const std::string& aSharedDataName, Plato::SharedData& aImportData);
    void exportData(const std::string& aSharedDataName, Plato::SharedData& aExportData);

    std::string getPerformerName() const;
    const std::string& getOperationName() const;
    std::vector<std::string> getInputDataNames() const;
    std::vector<std::string> getOutputDataNames() const;

    bool hasParameter(const std::string& aParamName) const;
    void setPerformer(std::shared_ptr<Performer> aPerformer);

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("OperationName", m_operationName);
        aArchive & boost::serialization::make_nvp("PerformerName", m_performerName);
        aArchive & boost::serialization::make_nvp("ArgumentNames", m_argumentNames);
        aArchive & boost::serialization::make_nvp("InputData", m_inputData);
        aArchive & boost::serialization::make_nvp("OutputData", m_outputData);
        aArchive & boost::serialization::make_nvp("Parameters", m_parameters);
    } 

    class Parameter : public Plato::SharedData {
      std::string m_name;
      std::string m_operation;
      double m_value = 0.0;
      public:
        Parameter() = default;
        Parameter(std::string name, std::string op, const double value=0.0) :
          m_name(std::move(name)), m_operation(std::move(op)), m_value(value){}

        // required (pure virtual in SharedData base class)
        int size() const override {return 1;}
        std::string myName() const override {return m_name;}
        std::string myContext() const override {return m_operation;}
        Plato::data::layout_t myLayout() const override
         {return Plato::data::layout_t::SCALAR_PARAMETER;}
        void transmitData() override {}
        void setData(const std::vector<double> & aData) override {m_value = aData[0];}
        void getData(std::vector<double> & aData) const override {aData[0] = m_value;}
        bool isDynamic() const override {return false;}

        template<class Archive>
        void serialize(Archive & aArchive, const unsigned int /*version*/)
        {
            aArchive & boost::serialization::make_nvp("SharedData",
                boost::serialization::base_object<SharedData>(*this));
            aArchive & boost::serialization::make_nvp("Name", m_name);
            aArchive & boost::serialization::make_nvp("Operation", m_operation);
            aArchive & boost::serialization::make_nvp("Value", m_value);
        }
    };

protected:

    /// @pre m_performer must not be `nullptr`.
    virtual void computeImpl();
    /// Called from setPerformer, this sets up the compute function needed by the constrained
    /// performer/application interface.
    /// @pre m_performer must not be `nullptr`
    virtual void setComputeFunctionOnNewPerformer(){}

    void addArgument(const std::string & tArgumentName,
                     const std::string & tSharedDataName,
                     const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData,
                     std::vector<std::shared_ptr<Plato::SharedData>>& aLocalData);

    static void addIfDoesNotExist(const std::shared_ptr<Plato::SharedData>& aDataToAdd,
        std::vector<std::shared_ptr<Plato::SharedData>>& aLocalData);

    std::map<std::string, std::shared_ptr<Plato::SharedData>> m_parameters;

    std::shared_ptr<Performer> m_performer;
    std::string m_performerName;
    std::string m_operationName;

    std::vector<std::shared_ptr<Plato::SharedData>> m_inputData;
    std::vector<std::shared_ptr<Plato::SharedData>> m_outputData;

    std::multimap<std::string, std::string> m_argumentNames;
};
} // End namespace Plato

#include <boost/serialization/assume_abstract.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Plato::Operation)

#endif
