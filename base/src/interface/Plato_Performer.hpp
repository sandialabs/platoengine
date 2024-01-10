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
 * Plato_Performer.hpp
 *
 *  Created on: April 23, 2017
 *
 */

#ifndef SRC_PERFORMER_HPP_
#define SRC_PERFORMER_HPP_

#include "Plato_OperationTypes.hpp"

#include "Plato_SerializationHeaders.hpp"

#include <string>
#include <vector>

namespace Plato
{

class Application;
class SharedData;

//!  Lightweight handle for storing Applications
/*!
 */
class Performer
{
public:
    Performer(){};
    Performer(const std::string & aMyName, const int & aCommID);

    void finalize();
    void compute(const std::string & aOperationName);


    void importData(const std::string & aArgumentName, const SharedData & aImportData);
    void exportData(const std::string & aArgumentName, SharedData & aExportData);

    void setApplication(Application* aApplication);
    Application * getApplication() const;

    const std::string& myName();
    int myCommID();

    //!{
    //! Constrained interface
    bool usesConstrainedOperationInterface() const;
    std::vector<OperationType> supportedOperationTypes() const;
    static auto computeFunction(OperationType aOperation) -> std::function<void(Performer&)>;
    //!}
    
    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("PerformerName",mName);
        //aArchive & boost::serialization::make_nvp("Application",mApplication);
        aArchive & boost::serialization::make_nvp("CommID",mCommID);
    } 

private:
    void computeCriterionValue();
    void computeCriterionGradient();
    void computeCriterionHessianTimesVector();

    Application* mApplication;  // TODO make this a unique pointer
    std::string mName;
    int mCommID;

private:
    Performer(const Performer& aRhs);
    Performer& operator=(const Performer& aRhs);
};

} // End namespace Plato

#endif
