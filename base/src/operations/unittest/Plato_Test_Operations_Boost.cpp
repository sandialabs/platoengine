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

#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <sstream>
#include <memory>
#include <Plato_FreeFunctions.hpp>

#include "Plato_CopyValue.hpp"
#include "Plato_SerializationHeaders.hpp"

#include "Plato_LocalOperation.hpp"

#include "Plato_Filter.hpp"
#include "Plato_CopyField.hpp"
#include "Plato_CopyValue.hpp"
#include "Plato_Roughness.hpp"

#include "Plato_SystemCallOperation.hpp"
#include "Plato_Aggregator.hpp"
#include "Plato_ChainRule.hpp"
#include "Plato_DesignVolume.hpp"
#include "Plato_Reinitialize.hpp"
#include "Plato_EnforceBounds.hpp"
#include "Plato_UpdateProblem.hpp"
#include "Plato_ComputeVolume.hpp"
#include "Plato_SIMP.hpp"
#include "Plato_CSMMeshOutput.hpp"
#include "Plato_SetUpperBounds.hpp"
#include "Plato_SetLowerBounds.hpp"
#include "Plato_PlatoMainOutput.hpp"
#include "Plato_InitializeField.hpp"
#include "Plato_InitializeValues.hpp"
#include "Plato_WriteGlobalValue.hpp"
#include "Plato_CSMParameterOutput.hpp"
#include "Plato_HarvestDataFromFile.hpp"
#include "Plato_OperationsUtilities.hpp"
#include "Plato_NormalizeObjectiveValue.hpp"
#include "Plato_MeanPlusVarianceMeasure.hpp"
#include "Plato_MeanPlusVarianceGradient.hpp"
#include "Plato_ReciprocateObjectiveValue.hpp"
#include "Plato_NormalizeObjectiveGradient.hpp"
#include "Plato_OutputNodalFieldSharedData.hpp"
#include "Plato_ReciprocateObjectiveGradient.hpp"

#include <boost/serialization/unique_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace Plato;

/*
    These tests verify that each individual object is being serialized and deserialized properly. 
    These tests DO NOT consider what set of objects specifically are being serialized.
*/

template<class Archive,class Serial>
void save(const Serial& tS, const std::string& aFilename)
{                                                                       
    std::ofstream tOutFileStream(aFilename.c_str());                    
    Archive tOutputArchive(tOutFileStream, boost::archive::no_header | boost::archive::no_tracking);       
    tOutputArchive << boost::serialization::make_nvp("Serial", tS);             
}         

template<class Archive, class Serial>
void load(Serial& tS, std::string aFilename)
{                                                                       
    std::fstream tInFileStream;
    tInFileStream.open(aFilename.c_str(),std::fstream::in);
    Archive tInputArchive(tInFileStream, 1);       
    tInputArchive >> boost::serialization::make_nvp("Serial", tS);
}        

template<class Serial>
bool serializeEquals(Serial& aSerialOne, Serial& aSerialTwo)
{
    save<boost::archive::text_oarchive>(aSerialOne,"out");
    save<boost::archive::text_oarchive>(aSerialTwo,"out2");

    std::ifstream tInfile("out");
    std::stringstream tStringstream;
    tStringstream << tInfile.rdbuf();
    auto tString = tStringstream.str();
    
    std::ifstream tInfile2("out2");
    std::stringstream tStringstream2;
    tStringstream2 << tInfile2.rdbuf();
    auto tString2 = tStringstream2.str();
    
    Plato::system("rm -rf out");
    Plato::system("rm -rf out2");

    return tString.compare(tString2) == 0;
}

class OpMap
{
public:
    std::map<std::string, std::unique_ptr<Plato::LocalOp>> mOperationMap;

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive &  boost::serialization::make_nvp("OperationMap",mOperationMap);
    }
};

Plato::FixedBlock::Metadata generateFixedBlockMetadata()
{
    Plato::FixedBlock::Metadata tMeta;
    tMeta.mOptimizationBlockValue = 1;
    tMeta.mBoundaryValues = {0.0, 0.1, 0.2};
    tMeta.mDomainValues = {-1.0, -2.0};
    tMeta.mMaterialStates = {"state1", "state2"};
    tMeta.mBlockIDs = {1 , 2 , 3, 4};
    tMeta.mSidesetIDs = {5, 6, 7};
    tMeta.mNodesetIDs = {8, 9};
    return tMeta;
}

TEST(BoostSerialization, SetLowerBounds)
{
    auto tFixedBlockMetadata = generateFixedBlockMetadata();

    Plato::SetLowerBounds tOperation("shape",
                                     "fluid",
                                     tFixedBlockMetadata,
                                     Plato::data::layout_t::SCALAR_FIELD,
                                     3,
                                     4);
    Plato::SetLowerBounds tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, SetUpperBounds)
{
    auto tFixedBlockMetadata = generateFixedBlockMetadata();

    Plato::SetUpperBounds tOperation("shape",
                                     "fluid",
                                     tFixedBlockMetadata,
                                     Plato::data::layout_t::SCALAR_FIELD,
                                     3,
                                     4);
    Plato::SetUpperBounds tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, CopyValue)
{
    Plato::CopyValue tOperation("in",
                                "out");
    Plato::CopyValue tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, CopyField)
{
    Plato::CopyField tOperation("in",
                                "out");
    Plato::CopyField tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, Aggregator)
{
    Plato::AggStruct tAggStruct;
    tAggStruct.mLayout = Plato::data::layout_t::SCALAR;
    tAggStruct.mOutputName = "Value";
    tAggStruct.mInputNames = {"Result1", "Result2", "Result3"};

    Plato::Aggregator tOperation({0.5, 0.5},
                                 {"BASES"},
                                 {"NORMALS"},
                                 {tAggStruct},
                                 "FIXED",
                                 2,
                                 true);
    Plato::Aggregator tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, ComputeVolume)
{
    Plato::ComputeVolume tOperation("Volume",
                                    "Volume Gradient",
                                    std::make_unique<SIMP>(2, 0.25),
                                    "Topology");
    Plato::ComputeVolume tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, DesignVolume)
{
    Plato::LocalArg tDefaultLocalArg;
    Plato::LocalArg tLocalArg(Plato::data::layout_t::SCALAR_FIELD, "Dummy", 4, true);
    
    Plato::DesignVolume tOperation({tDefaultLocalArg, tLocalArg});
    Plato::DesignVolume tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, Filter)
{
    Plato::AbstractFilter* tFilter = nullptr; //new Plato::AbstractFilter();
           
    Plato::Filter tOperation("Field","","Filtered Field",tFilter,true);
    Plato::Filter tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, PlatoMainOutput)
{
    Plato::LocalArg tLocalArg1(Plato::data::layout_t::SCALAR_FIELD, "topology", 0, true);
    Plato::LocalArg tLocalArg2(Plato::data::layout_t::SCALAR_FIELD, "control", 0, true);
    Plato::LocalArg tLocalArg3(Plato::data::layout_t::SCALAR_FIELD, "objective gradient", 0, true);
    Plato::LocalArg tLocalArg4(Plato::data::layout_t::SCALAR_FIELD, "constraint gradient", 0, true);
    Plato::LocalArg tLocalArg5(Plato::data::layout_t::SCALAR_FIELD, "dispx", 0, true);

    Plato::PlatoMainOutput tOperation("Iteration",
                                      "",
                                      "control",
                                      {"exo"},
                                      {tLocalArg1, tLocalArg2, tLocalArg3, tLocalArg4, tLocalArg5},
                                      2,
                                      1001,
                                      20,
                                      false,
                                      true);
    Plato::PlatoMainOutput tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, InitializeField)
{           
    Plato::InitializeField tOperation("File Name",
                                        "Uniform",
                                        "",
                                        "Initialized Field",
                                        "SPF",
                                        "X",
                                        "Y",
                                        "Z",
                                        "VN",
                                        std::array<double,3>{0,0,0},
                                        std::array<double,3>{1,1,1},
                                        std::vector<int>{0},
                                        Plato::data::layout_t::SCALAR,
                                        0.5,
                                        0,
                                        false);
    Plato::InitializeField tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, OperationsMap)
{     
    OpMap tOM;
    
    tOM.mOperationMap["Initialize"] = std::make_unique<Plato::InitializeField>(
                                        "File Name",
                                        "Uniform",
                                        "",
                                        "Initialized Field",
                                        "SPF",
                                        "X",
                                        "Y",
                                        "Z",
                                        "VN",
                                        std::array<double,3>{0.0, 0.0, 0.0},
                                        std::array<double,3>{1.0, 1.0, 1.0},
                                        std::vector<int>{0},
                                        Plato::data::layout_t::SCALAR,
                                        0.5,
                                        0,
                                        false);
    Plato::LocalArg tLocalArg1(Plato::data::layout_t::SCALAR_FIELD, "topology", 0, true);
    Plato::LocalArg tLocalArg2(Plato::data::layout_t::SCALAR_FIELD, "control", 0, true);
    Plato::LocalArg tLocalArg3(Plato::data::layout_t::SCALAR_FIELD, "objective gradient", 0, true);
    Plato::LocalArg tLocalArg4(Plato::data::layout_t::SCALAR_FIELD, "constraint gradient", 0, true);
    Plato::LocalArg tLocalArg5(Plato::data::layout_t::SCALAR_FIELD, "dispx", 0, true);

    tOM.mOperationMap["PlatoMainOutput"] = std::make_unique<Plato::PlatoMainOutput>("Iteration",
                                      "",
                                      "control",
                                      std::vector<std::string>{"exo"},
                                      std::vector<Plato::LocalArg>{tLocalArg1, tLocalArg2, tLocalArg3, tLocalArg4, tLocalArg5},
                                      2,
                                      1001,
                                      20,
                                      false,
                                      true);

    tOM.mOperationMap["Compute Volume"] = std::make_unique<Plato::ComputeVolume>("Volume",
                                    "Volume Gradient",
                                    std::make_unique<SIMP>(2, 0.25),
                                    "Topology");

    Plato::AggStruct tAggStruct;
    tAggStruct.mLayout = Plato::data::layout_t::SCALAR;
    tAggStruct.mOutputName = "Value";
    tAggStruct.mInputNames = {"Result1", "Result2", "Result3"};

    tOM.mOperationMap["Aggregator"] = std::make_unique<Plato::Aggregator>(
                                 std::vector<double>{0.5, 0.5},
                                 std::vector<std::string>{"BASES"},
                                 std::vector<std::string>{"NORMALS"},
                                 std::vector<Plato::AggStruct>{tAggStruct},
                                 "FIXED",
                                 2,
                                 true);

    tOM.mOperationMap["Copy Field"] = std::make_unique<Plato::CopyField>("in", "out");
    tOM.mOperationMap["Copy Value"] = std::make_unique<Plato::CopyValue>("in","out");
    
    auto tFixedBlockMetadata = generateFixedBlockMetadata();

    tOM.mOperationMap["Set Upper Bounds"] = std::make_unique<Plato::SetUpperBounds>("shape",
                                     "fluid",
                                     tFixedBlockMetadata,
                                     Plato::data::layout_t::SCALAR_FIELD,
                                     3,
                                     4);
                                     
    tOM.mOperationMap["Set Lower Bounds"] = std::make_unique<Plato::SetLowerBounds>("shape",
                                     "fluid",
                                     tFixedBlockMetadata,
                                     Plato::data::layout_t::SCALAR_FIELD,
                                     3,
                                     4);     

    Plato::LocalArg tDefaultLocalArg;
    Plato::LocalArg tLocalArg(Plato::data::layout_t::SCALAR_FIELD, "Dummy", 4, true);
    tOM.mOperationMap["Design Volume"] = std::make_unique<Plato::DesignVolume>(std::vector<Plato::LocalArg>{tDefaultLocalArg, tLocalArg});

    save<boost::archive::xml_oarchive>(tOM,"out.xml");
}

TEST(BoostSerialization, Roughness)
{           
    Plato::Roughness tOperation("Topology Name",
                                        "Roughness Name",
                                        "Gradient Name");
    Plato::Roughness tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}

TEST(BoostSerialization, WriteGlobalValue)
{           
    Plato::WriteGlobalValue tOperation("Input Name",
                                        "File Name",
                                       3);
    Plato::WriteGlobalValue tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}
/*
TEST(BoostSerialization, UpdateProblem)
{           
    //This test is somewhat of a placeholder, this operation doesn't seem to need anything.
    Plato::UpdateProblem tOperation();
    Plato::UpdateProblem tOperation2;
    
    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}
*/
TEST(BoostSerialization, EnforceBounds)
{
    Plato::EnforceBounds tOperation("Lower Bound Vector",
                                    "Upper Bound Vector",
                                    "Topology",
                                    "Clamped Topology");
    Plato::EnforceBounds tOperation2;
    EXPECT_FALSE(serializeEquals(tOperation,tOperation2));

    save<boost::archive::xml_oarchive>(tOperation,"out.xml");
    load<boost::archive::xml_iarchive>(tOperation2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tOperation,tOperation2));
}


TEST(BoostSerialization, ChainRule)
{
    Plato::ChainRule tChainRule("Chainy McChain Face",
                                "DFDX",
                                {"Some", "arbitrary", "strings"},
                                42);
    Plato::ChainRule tChainRule2;
    EXPECT_FALSE(serializeEquals(tChainRule,tChainRule2));

    save<boost::archive::xml_oarchive>(tChainRule,"out.xml");
    load<boost::archive::xml_iarchive>(tChainRule2,"out.xml");
    Plato::system("rm -rf out.xml");

    EXPECT_TRUE(serializeEquals(tChainRule, tChainRule2));
}
