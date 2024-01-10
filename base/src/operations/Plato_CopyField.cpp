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
 * Plato_CopyField.cpp
 *
 *  Created on: Jun 28, 2019
 */

#include "PlatoApp.hpp"
#include "Plato_Parser.hpp"
#include "Plato_CopyField.hpp"
#include "Plato_InputData.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::CopyField)

namespace Plato
{

CopyField::CopyField(PlatoApp* aPlatoApp, Plato::InputData& /*aNode*/) :
        Plato::LocalOp(aPlatoApp),
        mInputName("InputField"),
        mOutputName("OutputField")
{
}

CopyField::CopyField(const std::string& aInputName,
                     const std::string& aOutputName):
                     mInputName(aInputName),
                     mOutputName(aOutputName)
{
}

CopyField::~CopyField()
{
}

void CopyField::operator()()
{
    if(mPlatoApp->getTimersTree())
    {
        mPlatoApp->getTimersTree()->begin_partition(Plato::timer_partition_t::timer_partition_t::filter);
    }

    // get input data
    auto tInfield = mPlatoApp->getNodeField(mInputName);
    Real* tInputField;
    tInfield->ExtractView(&tInputField);
    auto tOutfield = mPlatoApp->getNodeField(mOutputName);
    Real* tOutputField;
    tOutfield->ExtractView(&tOutputField);

    // copy input field to output
    const int tLength = tInfield->MyLength();
    std::copy(tInputField, tInputField + tLength, tOutputField);

    if(mPlatoApp->getTimersTree())
    {
        mPlatoApp->getTimersTree()->end_partition();
    }
}

void CopyField::getArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    aLocalArgs.push_back(Plato::LocalArg
        { Plato::data::layout_t::SCALAR_FIELD, mInputName });
    aLocalArgs.push_back(Plato::LocalArg
        { Plato::data::layout_t::SCALAR_FIELD, mOutputName });
}

}
// namespace Plato
