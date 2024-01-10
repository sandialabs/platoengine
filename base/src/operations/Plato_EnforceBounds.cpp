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
 * Plato_EnforceBounds.cpp
 *
 *  Created on: Jun 29, 2019
 */

#include <algorithm>

#include "PlatoApp.hpp"
#include "Plato_InputData.hpp"
#include "Plato_EnforceBounds.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::EnforceBounds)

namespace Plato
{
    
EnforceBounds::EnforceBounds(const std::string& aLowerBoundVectorFieldName,
                            const std::string& aUpperBoundVectorFieldName,
                            const std::string& aTopologyFieldName,
                            const std::string& aTopologyOutputFieldName) :
                            mLowerBoundVectorFieldName(aLowerBoundVectorFieldName),
                            mUpperBoundVectorFieldName(aUpperBoundVectorFieldName),
                            mTopologyFieldName(aTopologyFieldName),
                            mTopologyOutputFieldName(aTopologyOutputFieldName)
{
}

EnforceBounds::EnforceBounds(PlatoApp* aPlatoApp, Plato::InputData& /*aNode*/) :
        Plato::LocalOp(aPlatoApp)
{
    mLowerBoundVectorFieldName = "Lower Bound Vector";
    mUpperBoundVectorFieldName = "Upper Bound Vector";
    mTopologyFieldName = "Topology";
    mTopologyOutputFieldName = "Clamped Topology";
}

void EnforceBounds::operator()()
{
    // Get the output field
    double* tOutputData;
    double* tInputData;
    double* tLowerBoundData;
    double* tUpperBoundData;
    int tDataLength = 0;

    auto& tInputField = *(mPlatoApp->getNodeField(mTopologyFieldName));
    tInputField.ExtractView(&tInputData);
    tDataLength = tInputField.MyLength();

    auto& tOutputField = *(mPlatoApp->getNodeField(mTopologyOutputFieldName));
    tOutputField.ExtractView(&tOutputData);
    
    memcpy(tOutputData, tInputData, tDataLength*sizeof(double));

    auto& tLowerBoundField = *(mPlatoApp->getNodeField(mLowerBoundVectorFieldName));
    tLowerBoundField.ExtractView(&tLowerBoundData);

    auto& tUpperBoundField = *(mPlatoApp->getNodeField(mUpperBoundVectorFieldName));
    tUpperBoundField.ExtractView(&tUpperBoundData);

    applyBounds(tDataLength, tLowerBoundData, tUpperBoundData, tOutputData);
}

void EnforceBounds::applyBounds(const int aDataLength,
                                const double *aLowerBoundData,
                                const double *aUpperBoundData,
                                double *aOutputData)
{
    for(int tIndex = 0; tIndex < aDataLength; tIndex++)
    {
        aOutputData[tIndex] = std::max(aOutputData[tIndex], aLowerBoundData[tIndex]);
        aOutputData[tIndex] = std::min(aOutputData[tIndex], aUpperBoundData[tIndex]);
    }
}

void EnforceBounds::getArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR_FIELD, mLowerBoundVectorFieldName));
    aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR_FIELD, mUpperBoundVectorFieldName));
    aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR_FIELD, mTopologyFieldName));
    aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR_FIELD, mTopologyOutputFieldName));
}

}
// namespace Plato
