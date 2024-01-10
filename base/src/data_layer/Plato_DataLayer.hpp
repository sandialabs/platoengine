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
 * Plato_DataLayer.hpp
 *
 *  Created on: May 1, 2017
 *
 */

#ifndef SRC_DATALAYER_HPP_
#define SRC_DATALAYER_HPP_

#include "Plato_SharedData.hpp"
#include "Plato_SerializationHeaders.hpp"

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>

#include <map>
#include <memory>
#include <vector>
#include <string>

namespace Plato
{
class SharedDataInfo;
struct CommunicationData;

/******************************************************************************/
//!  Data sharing manager
/*!
 */
/******************************************************************************/
class DataLayer
{
public:
    DataLayer() = default;
    DataLayer(const Plato::SharedDataInfo & aSharedDataInfo, const Plato::CommunicationData & aCommData);

    DataLayer(const Plato::DataLayer & aRhs) = delete;
    Plato::DataLayer & operator=(const Plato::DataLayer & aRhs) = delete;
    DataLayer(Plato::DataLayer && aRhs) = delete;
    Plato::DataLayer & operator=(Plato::DataLayer && aRhs) = delete;

    // accessors
    bool hasSharedData(const std::string& aName) const;
    SharedData& getSharedData(const std::string & aName) const;
    const std::vector<std::shared_ptr<SharedData>> & getSharedData() const;

    template<typename Archive>
    void serialize(Archive& aArchive, const unsigned int /*aVersion*/)
    {
        aArchive & boost::serialization::make_nvp("SharedDataVector", mSharedData);
        aArchive & boost::serialization::make_nvp("SharedDataMap", mSharedDataMap);
    }
    void initializeMPI(const Plato::CommunicationData& aCommData);

private:
    std::vector<std::shared_ptr<SharedData>> mSharedData;
    std::map<std::string, std::weak_ptr<SharedData>> mSharedDataMap;
};

} /* namespace Plato */

#endif /* SRC_INTERFACE_HPP_ */
