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
#include "Plato_Utils.hpp"
#include "PlatoApp.hpp"
#include "Plato_InputData.hpp"
#include "Plato_EnforceBounds.hpp"
#include "Plato_SetUpperBounds.hpp"
#include "Plato_SystemCallOperation.hpp"
#include "Plato_OperationsUtilities.hpp"
#ifdef STK_ENABLED
#include "stk_io/StkMeshIoBroker.hpp"
#endif
#include <fstream>
#include <gtest/gtest.h>

namespace PlatoTestOperations
{

#ifdef STK_ENABLED
namespace
{

Plato::InputData create_input_data_for_upper_bounds_operation()
{
    std::stringstream tBuffer;
    tBuffer << "<Operation>" << std::endl;
    tBuffer << "<Function>SetUpperBounds</Function>" << std::endl;
    tBuffer << "<Name>Compute Upper Bounds</Name>" << std::endl;
    tBuffer << "<UseCase>solid</UseCase>" << std::endl;
    tBuffer << "<Discretization>density</Discretization>" << std::endl;
    tBuffer << "<Input>" << std::endl;
    tBuffer << "  <ArgumentName>Upper Bound Value</ArgumentName>" << std::endl;
    tBuffer << "</Input>" << std::endl;
    tBuffer << "<Output>" << std::endl;
    tBuffer << "  <ArgumentName>Upper Bound Vector</ArgumentName>" << std::endl;
    tBuffer << "</Output>" << std::endl;
    tBuffer << "<FixedBlocks>" << std::endl;
    tBuffer << "  <Index>1</Index>" << std::endl;
    tBuffer << "  <DomainValue>.5</DomainValue>" << std::endl;
    tBuffer << "  <BoundaryValue>.25</BoundaryValue>" << std::endl;
    tBuffer << "</FixedBlocks>" << std::endl;
    tBuffer << "</Operation>" << std::endl;
    Plato::PugiParser tParser;
    return tParser.parseString(tBuffer.str());
}

void create_two_block_hex_mesh(const std::string &aMeshFilename)
{
    stk::io::StkMeshIoBroker tIoBroker(MPI_COMM_WORLD);
    tIoBroker.use_simple_fields();
    tIoBroker.add_mesh_database("textmesh:0,1,HEX_8,1,2,3,4,5,6,7,8,block_1\n0,2,HEX_8,5,6,7,8,9,10,11,12,block_2|coordinates: 0,0,0, 1,0,0, 1,1,0, 0,1,0, 0,0,1, 1,0,1, 1,1,1, 0,1,1, 0,0,2, 1,0,2, 1,1,2, 0,1,2", stk::io::READ_MESH);
    tIoBroker.create_input_mesh();
    tIoBroker.populate_bulk_data();
    Ioss::PropertyManager tProperties;
    const size_t tOutputFileIndex = tIoBroker.create_output_mesh(aMeshFilename, stk::io::WRITE_RESULTS, tProperties);
    tIoBroker.write_output_mesh(tOutputFileIndex);
}

void create_plato_main_input_deck(const std::string &aInputDeckFilename)
{
    std::ofstream tInputDeckStream;
    tInputDeckStream.open(aInputDeckFilename);
    tInputDeckStream << "<?xml version=\"1.0\"?>" << std::endl;
    tInputDeckStream << "<mesh>" << std::endl;
    tInputDeckStream << "<type>unstructured</type>" << std::endl;
    tInputDeckStream << "<format>exodus</format>" << std::endl;
    tInputDeckStream << "<ignore_node_map>true</ignore_node_map>" << std::endl;
    tInputDeckStream << "<ignore_element_map>true</ignore_element_map>" << std::endl;
    tInputDeckStream << "<mesh>temp_mesh.exo</mesh>" << std::endl;
    tInputDeckStream << "</mesh>" << std::endl;
    tInputDeckStream.close();
}

void create_plato_main_operations_file(const std::string &aOperationsFilename)
{
    std::ofstream tOperationsFile;
    tOperationsFile.open(aOperationsFilename);
    // Just add something so it isn't empty
    tOperationsFile << "<?xml version=\"1.0\"?>" << std::endl;
    tOperationsFile << "<Filter>" << std::endl;
    tOperationsFile << "<Name>Kernel</Name>" << std::endl;
    tOperationsFile << "<Scale>1e-10</Scale>" << std::endl;
    tOperationsFile << "<Power>1</Power>" << std::endl;
    tOperationsFile << "</Filter>" << std::endl;
    tOperationsFile.close();
}

} // end anonymous namespace
#endif

TEST(EnforceBounds, applyBounds)
{
    Plato::InputData tNode;
    Plato::EnforceBounds tEnforceBounds(NULL, tNode);

    int tLength = 10;
    double tDataToBound[10] = {1, -3, 5, 10, 2, 7, 22, 33, 0, 0};
    double tLowerBound[10] =  {1.5, 0, 5, -10, 3, 7, -1, 14, -1, -1};
    double tUpperBound[10] =  {2, 1, 5, 1, 3.3, 7.5, 0, 15, 0, 1};

    tEnforceBounds.applyBounds(tLength, tLowerBound, tUpperBound, tDataToBound); 

    EXPECT_EQ(tDataToBound[0], 1.5);
    EXPECT_EQ(tDataToBound[1], 0);
    EXPECT_EQ(tDataToBound[2], 5);
    EXPECT_EQ(tDataToBound[3], 1);
    EXPECT_EQ(tDataToBound[4], 3);
    EXPECT_EQ(tDataToBound[5], 7);
    EXPECT_EQ(tDataToBound[6], 0);
    EXPECT_EQ(tDataToBound[7], 15);
    EXPECT_EQ(tDataToBound[8], 0);
    EXPECT_EQ(tDataToBound[9], 0);
}

#ifdef STK_ENABLED
TEST(SetUpperBounds, updateUpperBoundsBasedOnFixedEntitiesForDBTOP)
{
    // Create a mesh with two hex elements, each in its own block
    std::string tMeshFilename = "temp_mesh.exo";
    create_two_block_hex_mesh(tMeshFilename);

    // Create an input deck and operations files for PlatoMain app
    std::string tInputDeckFilename = "platomain.xml";
    create_plato_main_input_deck(tInputDeckFilename);
    std::string tOperationsFilename = "operations.xml";
    create_plato_main_operations_file(tOperationsFilename);
    
    // Create a PlatoMain app to be used when instantiating the SetUpperBounds operation
    constexpr int argc = 1;
    char exeName[] = "exeName";
    char* argv[1] = {exeName};
    MPI_Comm tComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &tComm);
    setenv("PLATO_APP_FILE", tOperationsFilename.c_str(), true);
    auto tApp = std::make_shared<PlatoApp>(argc, argv, tComm);
    tApp->initialize();

    // Create some input for the SetUpperBounds operation
    const Plato::InputData tInputData = create_input_data_for_upper_bounds_operation();

    // Creat the SetUpperBounds operation
    auto tOperation = tInputData.get<Plato::InputData>("Operation");
    Plato::SetUpperBounds tUpperBounds{tApp.get(), tOperation};
    constexpr int tTotalNumNodesInMesh = 12;
    constexpr double tDefaultUpperBound = 1.0;
    std::vector<double> tUpperBoundValues(tTotalNumNodesInMesh, tDefaultUpperBound);

    // Call the function we are testing
    tUpperBounds.updateUpperBoundsBasedOnFixedEntitiesForDBTOP(tUpperBoundValues.data());
    const std::vector<double> tGoldValues = {0.5, 0.5, 0.5, 0.5, 0.25, 0.25, 0.25, 0.25, 1, 1, 1, 1};

    EXPECT_EQ(tUpperBoundValues, tGoldValues);

    // Clean up after ourselves
    std::remove(tMeshFilename.c_str());
    std::remove(tInputDeckFilename.c_str());
    std::remove(tOperationsFilename.c_str());
}


#endif

} // end PlatoTestOperations namespace
