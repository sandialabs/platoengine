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
 * Plato_Filter.cpp
 *
 *  Created on: Jun 28, 2019
 */

#include "PlatoApp.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Filter.hpp"
#include "Plato_InputData.hpp"
#include "PlatoEngine_AbstractFilter.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::Filter)

namespace Plato
{

Filter::Filter(const std::string& aInputToFilterName,
               const std::string& aInputBaseFieldName,
               const std::string& aOutputFromFilterName,
               Plato::AbstractFilter* aFilter,
               bool aIsGradient) :
               mFilter(aFilter),
               mInputToFilterName(aInputToFilterName),
               mInputBaseFieldName(aInputBaseFieldName),
               mOutputFromFilterName(aOutputFromFilterName),
               mIsGradient(aIsGradient)
{
}


Filter::Filter(PlatoApp* aPlatoApp, Plato::InputData& aNode) :
        Plato::LocalOp(aPlatoApp),
        mFilter(),
        mInputToFilterName(),
        mInputBaseFieldName(),
        mOutputFromFilterName(),
        mIsGradient()
{
    // retrieve filter
    mFilter = mPlatoApp->getFilter();

    // decide names differently
    mIsGradient = Plato::Get::Bool(aNode, "Gradient");
    if(mIsGradient == true)
    {
        mInputToFilterName = "Gradient";
        mInputBaseFieldName = "Field";
        mOutputFromFilterName = "Filtered Gradient";
    }
    else
    {
        mInputToFilterName = "Field";
        mInputBaseFieldName = "";
        mOutputFromFilterName = "Filtered Field";
    }
}

Filter::~Filter()
{
    mFilter = nullptr;
}

void Filter::operator()()
{
    if(mPlatoApp->getTimersTree())
    {
        mPlatoApp->getTimersTree()->begin_partition(Plato::timer_partition_t::timer_partition_t::filter);
    }

    // get input data
    auto tInfield = mPlatoApp->getNodeField(mInputToFilterName);
    Real* tInputField;
    tInfield->ExtractView(&tInputField);
    auto tOutfield = mPlatoApp->getNodeField(mOutputFromFilterName);
    Real* tOutputField;
    tOutfield->ExtractView(&tOutputField);

    // copy input field to output
    const int tLength = tInfield->MyLength();
    std::copy(tInputField, tInputField + tLength, tOutputField);

    if(mIsGradient == true)
    {
        // get base field for gradient application
        auto tBasefield = mPlatoApp->getNodeField(mInputBaseFieldName);
        Real* tBaseField;
        tBasefield->ExtractView(&tBaseField);

        if(mFilter)
        {
            mFilter->apply_on_gradient(tLength, tBaseField, tOutputField);
        }
    }
    else
    {
        if(mFilter)
        {
            mFilter->apply_on_field(tLength, tOutputField);
        }
    }

    if(mPlatoApp->getTimersTree())
    {
        mPlatoApp->getTimersTree()->end_partition();
    }
}

void Filter::getArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    aLocalArgs.push_back(Plato::LocalArg
        { Plato::data::layout_t::SCALAR_FIELD, mInputToFilterName });
    if(!mInputBaseFieldName.empty())
    {
        aLocalArgs.push_back(Plato::LocalArg
            { Plato::data::layout_t::SCALAR_FIELD, mInputBaseFieldName });
    }
    aLocalArgs.push_back(Plato::LocalArg
        { Plato::data::layout_t::SCALAR_FIELD, mOutputFromFilterName });
}

}
// namespace Plato
