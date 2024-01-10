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

#include "Plato_ESP.hpp"
#include "Plato_KokkosTypes.hpp"
#include "Plato_Exceptions.hpp"

namespace Plato {
namespace Geometry {

template <typename ScalarType, typename ScalarVectorType>
ESP<ScalarType,ScalarVectorType>::ESP(std::string aModelFileName, std::string aTessFileName, int aParameterIndex) :
    mModelFileName(aModelFileName),
    mTessFileName(aTessFileName)
{
    readNames();

    if(aParameterIndex != -1){
        auto tName = mParameterNames[aParameterIndex];
        mParameterNames.resize(1);
        mParameterNames[0] = tName;
    }

    mSensitivity.resize(mParameterNames.size());
}

template <typename ScalarType, typename ScalarVectorType>
ScalarType ESP<ScalarType,ScalarVectorType>::sensitivity(int aParameterIndex, ScalarVectorType aGradientX)
{
    ScalarType tDfDp(0.0);
    auto& tSensitivity = mSensitivity[aParameterIndex];
    int tNumData = tSensitivity.size();
    for (int k=0; k<tNumData; k++)
    {
        tDfDp += tSensitivity[k]*aGradientX[k];
    }
    return tDfDp;
}

template <typename ScalarType, typename ScalarVectorType>
void ESP<ScalarType,ScalarVectorType>::throwWithError(std::string aError)
{
    throw Plato::ParsingException(aError);
}

template <typename ScalarType, typename ScalarVectorType>
void ESP<ScalarType,ScalarVectorType>::readNames()
{
    mParameterNames.clear();
    std::ifstream tInputStream;
    tInputStream.open(mModelFileName.c_str());
    if(tInputStream.good())
    {
        std::string tLine, tWord;
        while(std::getline(tInputStream, tLine))
        {
            std::istringstream tStream(tLine, std::istringstream::in);
            while( tStream >> tWord )    
            {
                for(auto& c : tWord) { c = std::tolower(c); }
                if (tWord == "despmtr")
                {
                    tStream >> tWord;
                    mParameterNames.push_back(tWord);
                }
            }
        }
        tInputStream.close();
    }
}

template class ESP<double,std::vector<double>>;
template class ESP<double,Plato::ScalarVectorT<double>::HostMirror>;

} // end namespace Geometry
} // end namespace Plato
