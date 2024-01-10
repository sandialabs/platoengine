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
 * Plato_PlatoProxyApp.hpp
 *
 *  Created on: Oct 17, 2017
 */

#ifndef PLATO_PLATOPROXYAPP_HPP_
#define PLATO_PLATOPROXYAPP_HPP_

#include <mpi.h>

#include "Plato_Application.hpp"

namespace Plato
{

class SharedData;

class PlatoProxyApp : public Plato::Application
{
public:
    PlatoProxyApp(const std::string & aInputMeshFile, const MPI_Comm & aAppComm);
    ~PlatoProxyApp() override;

    void exportDataMap(const Plato::data::layout_t & aDataLayout, std::vector<int> & aMyOwnedGlobalIDs) override;

    void finalize() override;
    void initialize() override;
    void reinitialize() override;
    void compute(const std::string & aOperationName) override;
    void exportData(const std::string & aArgumentName, Plato::SharedData& aExportData) override;
    void importData(const std::string & aArgumentName, const Plato::SharedData& aImportData) override;

    void getSubDomainOwnedGlobalIDs(std::vector<int> & aInput) const;
    void getSubDomainOwnedAndSharedGlobalIDs(std::vector<int> & aInput) const;

private:
    MPI_Comm mAppComm;
    std::string mInputMeshFile;

    std::vector<double> mObjectiveValue;
    std::vector<double> mDesignVariables;
    std::vector<double> mObjectiveGradient;

    std::vector<int> mGlobalIDsOwned;
    std::vector<int> mGlobalIDsOwnedAndShared;

private:
    PlatoProxyApp(const Plato::PlatoProxyApp& aRhs);
    Plato::PlatoProxyApp& operator=(const Plato::PlatoProxyApp& aRhs);
};

} // namespace Plato

#endif /* PLATO_PLATOPROXYAPP_HPP_ */
