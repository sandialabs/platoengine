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
 * Plato_Stage.hpp
 *
 *  Created on: April 23, 2017
 *
 */

#ifndef SRC_STAGE_HPP_
#define SRC_STAGE_HPP_

#include <string>
#include <vector>
#include <memory>

#include <boost/serialization/shared_ptr.hpp>

#include "Plato_Operation.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_SerializationHeaders.hpp"

namespace Plato
{
class Performer;
class StageInputDataMng;

//! Sequence of Operations that correspond to a call to Plato::Interface::compute()
/*!
 */
class Stage final
{
public:
    Stage() = default;
    Stage(const Plato::StageInputDataMng & aStageInputData,
          const std::shared_ptr<Plato::Performer> aPerformer,
          const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData);

    void update(const Plato::StageInputDataMng & aStageInputData,
		const std::shared_ptr<Plato::Performer> aPerformer,
		const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData);

    Plato::Operation* getNextOperation();
    void begin();
    void end();

    std::string getName() const
    {
        return m_name;
    }

    std::vector<std::string> getInputDataNames() const;
    std::vector<std::string> getOutputDataNames() const;

    void setPerformerOnOperations(std::shared_ptr<Performer> aPerformer);

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("StageName",m_name);
        aArchive & boost::serialization::make_nvp("Operations",m_operations);
        aArchive & boost::serialization::make_nvp("InputData",m_inputData);
        aArchive & boost::serialization::make_nvp("OutputData",m_outputData);
        aArchive & boost::serialization::make_nvp("CurrentOperationIndex",currentOperationIndex);
    }

    /// @return `true` if any operation on this stage has a parameter with name @a aParameterName
    bool hasParameter(const std::string& aParameterName) const;

private:
    void initializeSharedData(const Plato::StageInputDataMng & aStageInputData,
                              const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData);

    std::string m_name;
    std::vector<std::unique_ptr<Operation>> m_operations;
    std::vector<std::shared_ptr<Plato::SharedData>> m_inputData;
    std::vector<std::shared_ptr<Plato::SharedData>> m_outputData;

    int currentOperationIndex = 0;
};

} // End namespace Plato

#endif
