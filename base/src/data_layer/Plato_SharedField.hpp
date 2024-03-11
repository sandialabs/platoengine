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
 * Plato_SharedField.hpp
 *
 *  Created on: April 23, 2017
 *
 */

#ifndef SRC_SHAREDFIELD_HPP_
#define SRC_SHAREDFIELD_HPP_

#include <string>
#include <vector>
#include <memory>

#include <Epetra_Vector.h>
#include <Epetra_MpiComm.h>
#include <Epetra_Map.h>
#include <Epetra_Import.h>

#include "Plato_SharedData.hpp"
#include "Plato_Communication.hpp"
#include "Plato_SharedDataInfo.hpp"

#include "Plato_SerializationHeaders.hpp"

namespace Plato
{

struct communication;
struct CommunicationData;

class SharedField : public SharedData
{
public:
    SharedField(){};
    SharedField(const std::string & aMyName,
                const Plato::communication::broadcast_t & aMyBroadcast,
                const Plato::CommunicationData & aCommData,
                Plato::data::layout_t aMyLayout);

    int size() const override;
    std::string myName() const override;
    Plato::data::layout_t myLayout() const override;
    Plato::communication::broadcast_t myBroadcast() const;

    void transmitData() override;
    void setData(const std::vector<double> & aData) override;
    void getData(std::vector<double> & aData) const override;

    void setData(const double & aDataVal, const int & aGlobalIndex);
    void getData(double & dataVal, const int & aGlobalIndex) const;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("SharedData", boost::serialization::base_object<SharedData>(*this));
        aArchive & boost::serialization::make_nvp("SharedFieldName",mMyName);
        aArchive & boost::serialization::make_nvp("Layout",mMyLayout);
        aArchive & boost::serialization::make_nvp("Broadcast",mMyBroadcast);
    }
    void initializeMPI(const Plato::CommunicationData& aCommData) override;
    bool isDynamic() const override;
private:
    void initialize(const Plato::CommunicationData & aCommData);

private:
    std::string mMyName;
    Plato::data::layout_t mMyLayout;
    Plato::communication::broadcast_t mMyBroadcast;

    std::shared_ptr<Epetra_MpiComm> mEpetraComm;
    std::shared_ptr<Epetra_Map> mGlobalIDsProvided;
    std::shared_ptr<Epetra_Map> mGlobalIDsReceived;

    std::shared_ptr<Epetra_Import> mNodeImporter;

    std::shared_ptr<Epetra_Vector> mSendDataVector;
    std::shared_ptr<Epetra_Vector> mRecvDataVector;

private:
    SharedField(const SharedField& aRhs);
    SharedField& operator=(const SharedField& aRhs);
};

} // End namespace Plato

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY2(Plato::SharedField, "SharedField")

#endif
