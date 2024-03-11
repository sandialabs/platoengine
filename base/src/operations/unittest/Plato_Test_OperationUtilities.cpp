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
#include "Plato_InputData.hpp"
#ifdef STK_ENABLED
#include "stk_io/StkMeshIoBroker.hpp"
#include "Ioss_NodeBlock.h"
#endif
#include "Plato_OperationsUtilities.hpp"
#include "Plato_Exceptions.hpp"
#include <gtest/gtest.h>
#include <cstdio>

namespace PlatoTestOperationUtilities
{

TEST(PlatoTestOperationUtilities, SingleStringPath_Success)
{
    // Setup an input node for creating the operation
    Plato::InputData tNode("Operation");
    tNode.add<std::string>("MeshFilename", "temp_mesh.exo");
    
    ASSERT_STREQ(findFirstStringParameter({"MeshFilename"}, tNode).c_str(), "temp_mesh.exo");
}

TEST(PlatoTestOperationUtilities, DoubleStringPath_Success)
{
    // Setup an input node for creating the operation
    Plato::InputData tNode("Operation");
    Plato::InputData tOutput("Output");
    tOutput.add<std::string>("ArgumentName", "node_ids");
    tNode.add<Plato::InputData>("Output", tOutput);
    
    ASSERT_STREQ(findFirstStringParameter({"Output","ArgumentName"}, tNode).c_str(), "node_ids");
}

TEST(PlatoTestOperationUtilities, InvalidPath_Throw)
{
    // Setup an input node for creating the operation
    Plato::InputData tNode("Operation");
    Plato::InputData tOutput("Output");
    tOutput.add<std::string>("ArgumentName", "node_ids");
    tNode.add<Plato::InputData>("Output", tOutput);
    
    EXPECT_THROW(findFirstStringParameter({"Output_mispelled", "ArgumentName"}, tNode), Plato::ParsingException/*std::exception*/);
}

TEST(PlatoTestOperationUtilities, InvalidFinalString_EmptyResult)
{
    // Setup an input node for creating the operation
    Plato::InputData tNode("Operation");
    Plato::InputData tOutput("Output");
    tOutput.add<std::string>("ArgumentName", "node_ids");
    tNode.add<Plato::InputData>("Output", tOutput);
    
    ASSERT_STREQ(findFirstStringParameter({"Output", "GarbageName"}, tNode).c_str(), "");
}

TEST(PlatoTestOperationUtilities, EmptyPath_Throw)
{
    // Setup an input node for creating the operation
    Plato::InputData tNode("Operation");
    Plato::InputData tOutput("Output");
    tOutput.add<std::string>("ArgumentName", "node_ids");
    tNode.add<Plato::InputData>("Output", tOutput);
    
    EXPECT_THROW(findFirstStringParameter({}, tNode), Plato::ParsingException/*std::exception*/);
}

#ifdef STK_ENABLED

TEST(PlatoTestOperationUtilities, ExtractGlobalNodeIDs_succeed_trivial_node_map)
{
    stk::io::StkMeshIoBroker iobroker(MPI_COMM_WORLD);
    iobroker.use_simple_fields();
    iobroker.add_mesh_database("generated:1x1x1", stk::io::READ_MESH);
    iobroker.create_input_mesh();
    iobroker.populate_bulk_data();
    Ioss::PropertyManager properties;
    size_t outputFileIndex = iobroker.create_output_mesh("temp_mesh.exo", stk::io::WRITE_RESULTS, properties);
    iobroker.write_output_mesh(outputFileIndex);
  
    // Call the function to extract the node ids from the mesh on disk
    std::vector<unsigned int> tResults = Plato::extractGlobalNodeIDs(MPI_COMM_WORLD, "temp_mesh.exo");
    
    // Test against gold values
    const std::vector<unsigned int> tGold = {1,2,3,4,5,6,7,8};
    EXPECT_EQ(tGold, tResults);
    
    // Clean up mesh from disk
    std::remove("temp_mesh.exo");
}

TEST(PlatoTestOperationUtilities, ExtractGlobalNodeIDs_succeed_non_trivial_node_map)
{
    const std::vector<unsigned int> tGold = {1, 2, 3, 4, 11, 12, 13, 14};
    
    stk::io::StkMeshIoBroker iobroker(MPI_COMM_WORLD);
    iobroker.use_simple_fields();
    iobroker.add_mesh_database("textmesh:0,1,HEX_8,1,2,3,4,11,12,13,14", stk::io::READ_MESH);
    iobroker.create_input_mesh();
    iobroker.populate_bulk_data();
    Ioss::PropertyManager properties;
    const size_t outputFileIndex = iobroker.create_output_mesh("temp_mesh.exo", stk::io::WRITE_RESULTS, properties);
    iobroker.write_output_mesh(outputFileIndex);

    // Call the function to extract the node ids from the mesh on disk
    const std::vector<unsigned int> tResults = Plato::extractGlobalNodeIDs(MPI_COMM_WORLD, "temp_mesh.exo");
    
    // Test against gold values
    EXPECT_EQ(tResults, tGold);
    
    // Clean up mesh from disk
    std::remove("temp_mesh.exo");
}

#endif
    
} // end PlatoTestOperationUtilities namespace
