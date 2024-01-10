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
 * Plato_ChainRule.cpp
 *
 *  Created on: Dec 12, 2022
 */

#include <fstream>
#include "PlatoApp.hpp"
#include "Plato_Parser.hpp"
#include "Plato_ChainRule.hpp"
#include "Plato_InputData.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::ChainRule)

namespace Plato
{

ChainRule::ChainRule(std::string aOutputName,
    std::string aDFDXName,
    std::vector<std::string> aInputNames,
    unsigned int aSpatialDims) :
    mOutputName(std::move(aOutputName)),
    mDFDXName(std::move(aDFDXName)),
    mInputNames(std::move(aInputNames)),
    mSpatialDims(aSpatialDims)
{}

ChainRule::ChainRule(PlatoApp* aPlatoApp, Plato::InputData& aNode) :
        Plato::LocalOp(aPlatoApp)
{
    for( auto tInputNode : aNode.getByName<Plato::InputData>("Input") )
    {
        auto tName = Plato::Get::String(tInputNode, "ArgumentName");
        if(tName == "DFDX")
        {
            mDFDXName = tName;
        }
        else
        {
            mInputNames.push_back(tName);
        }
    }
    auto tOutputNode = aNode.getByName<Plato::InputData>("Output");
    if(tOutputNode.size() != 0)
    { 
        mOutputName = Plato::Get::String(tOutputNode[0], "ArgumentName");
    }

    if( aNode.size<std::string>("Dimensions") )
    {
        mSpatialDims = Plato::Get::Int(aNode, "Dimensions");
    }
}

void ChainRule::parseSensitivityMap(std::vector<unsigned int> &aLocalToGlobalNodeIDMap)
{
    std::fstream tFileStream("ESP_Mesh/Scratch/plato/sensMap.txt");
    if(!tFileStream.good())
    {
        THROWERR(std::string("Error opening ESP_Mesh/Scratch/plato/sensMap.txt in ChainRule operation.\n"))
    }
/*
    std::vector<int> tNumNodesInBlock(2);
    int tNumNodesets = mPlatoApp->getLightMP()->getMesh()->getNumNodeSets();
    for(int ib=0; ib<tNumNodesets; ib++)
    {
        DMNodeSet *nsi = mPlatoApp->getLightMP()->getMesh()->getNodeSet(ib);
        if(nsi->id == 100)
          tNumNodesInBlock[0] = nsi->numNodes;
        else if(nsi->id == 200)
          tNumNodesInBlock[1] = nsi->numNodes;
    }
*/

//unsigned int ctr_debug = 1;
    unsigned int tNumBlocks;
    tFileStream >> tNumBlocks;
    for(unsigned int i=0; i<tNumBlocks; ++i)
    {
        unsigned int tNumNodes;
        tFileStream >> tNumNodes;
        std::vector<unsigned int> tCurBlockNodes(tNumNodes);
        for(unsigned int j=0; j<tNumNodes; ++j) 
        {
            unsigned int tGlobalNodeID; 
            tFileStream >> tGlobalNodeID;
 //           tCurBlockNodes[j] = ctr_debug++;
            tCurBlockNodes[j] = tGlobalNodeID;
        }
  //      ctr_debug += (tNumNodesInBlock[i]-tNumNodes);
        aLocalToGlobalNodeIDMap.insert(aLocalToGlobalNodeIDMap.end(), tCurBlockNodes.begin(), tCurBlockNodes.end());
    }

    tFileStream.close();
}

/* This operation assumes that the dFdX data is coming in a vector that is num_nodes*3 long
 * and is indexed by global_node_id-1 where the dFdX of node with global id 1 occupies 
 * locations 0, 1, and 2 in the vector (for x, y, and z components dFdX).
 */
void ChainRule::operator()()
{
    if(mPlatoApp->getTimersTree())
    {
        mPlatoApp->getTimersTree()->begin_partition(Plato::timer_partition_t::timer_partition_t::filter);
    }

    std::vector<unsigned int> tLocalToGlobalNodeIDMap;
    parseSensitivityMap(tLocalToGlobalNodeIDMap);

    std::vector<double>& tOutputVector = *(mPlatoApp->getValue(mOutputName));
    tOutputVector.resize(mInputNames.size());
    const auto& tDFDX = *(mPlatoApp->getValue(mDFDXName));
    unsigned int tLargestAllowableDFDXGlobalNodeID = tDFDX.size()/mSpatialDims;

    bool tFirstTime = true;
    unsigned int tNumMapEntries=tLocalToGlobalNodeIDMap.size();
    unsigned int tNumNodes = tNumMapEntries;
    unsigned int tNumSensEntries=0;
    unsigned int tEntryIndex = 0;

    unsigned int tESPSpatialDims = 3;

    for( const auto& tInputName : mInputNames )
    {
        const auto& tCurDXDP = *(mPlatoApp->getValue(tInputName));
        if(tFirstTime)
        {
            tNumSensEntries = tCurDXDP.size();
            if( tNumSensEntries/tESPSpatialDims != tNumNodes ) 
            {
                THROWERR(std::string("Mismatch between number of sensitivities and sensitivity map in ChainRule operation.\n"))
            }
            tFirstTime = false;
        }
        double tValue(0.0);
        for( unsigned int tIndex=0; tIndex<tNumNodes; tIndex++)
        {
            unsigned int tLocalIndex = tIndex*tESPSpatialDims;
            unsigned int tGlobalNodeID = tLocalToGlobalNodeIDMap[tIndex];
            if(tGlobalNodeID <= tLargestAllowableDFDXGlobalNodeID)
            {
                unsigned int tGlobalIndex = (tLocalToGlobalNodeIDMap[tIndex]-1)*mSpatialDims;
                for(unsigned tSpatialDim = 0; tSpatialDim < mSpatialDims; tSpatialDim++)
                {
                    tValue += tCurDXDP[tLocalIndex+tSpatialDim]*tDFDX[tGlobalIndex+tSpatialDim];
                }
            }
        }
        tOutputVector[tEntryIndex++] = tValue;
    }
}

void ChainRule::getArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    for( const auto& tInputName : mInputNames )
    {
        aLocalArgs.push_back(Plato::LocalArg
            { Plato::data::layout_t::SCALAR, tInputName });
    }
    aLocalArgs.push_back(Plato::LocalArg
        { Plato::data::layout_t::SCALAR, mOutputName });
    aLocalArgs.push_back(Plato::LocalArg
        { Plato::data::layout_t::SCALAR, mDFDXName });
}

}
// namespace Plato
