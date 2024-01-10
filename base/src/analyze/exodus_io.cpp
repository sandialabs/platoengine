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

#include "topological_element.hpp"
#include "exception_handling.hpp"
#include "types.hpp"
#include "mesh_io.hpp"
#include "communicator.hpp"
#include "data_container.hpp"
#include "data_mesh.hpp"

#include "exodusII.h"

#include <iostream>
#include <fstream>
#include <string>
using std::ifstream;
using std::string;
using std::cout;
using std::endl;
using std::vector;

ExodusIO::ExodusIO() :
        MeshIO()
{
    myType = READ;
    myName = "ExodusIO";
    myTitle = "Fester ExodusIO";
    myFileID = -1;
    myMesh = NULL;
    myData = NULL;
}

ExodusIO::~ExodusIO() 
{
   closeMeshIO();
}

bool
ExodusIO::openMeshIO()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  int CPU_word_size = 8;
  int IO_word_size = 8;
  char name[80];
  float version = 0.;

  sprintf(name, "%s", myName.c_str());
  int temp = testFile(name);
  if( (myType == CLOBBER) || ( temp >= 0) ) {
    if( myType == READ ) {
      myFileID = ex_open(name,EX_READ,&CPU_word_size,&IO_word_size,&version);
      if( myFileID <= 0 ) {
	pXcout << "!!! Problem opening exodus file " << name << "\n"
               << "!!! \tfor reading.\n" << endl;
        return false;
      } else
        return true;
    } else if( myType == CLOBBER ) {
      myFileID = ex_create(name,EX_CLOBBER,&CPU_word_size,&IO_word_size);
      if( myFileID <= 0 ) {
	pXcout << "!!! Problem opening exodus file " << name << "\n"
               << "!!! \tfor clobber.\n" << endl;
        return false;
      } else
        return true;
    } else if( myType == WRITE ) {
      myFileID = ex_open(name,EX_WRITE,&CPU_word_size,&IO_word_size,&version);
      if( myFileID <= 0 ) {
	pXcout << "!!! Problem opening exodus file " << name << "\n" 
               << "!!! \tfor write. File may already exist.\n" << endl;
        return false;
      } else
        return true;
    }
  }
  pXcout << "!!! Problem with exodus file " << name << "\n"
	 << "!!! \tnon-existent or non-netcdf format\n" << endl;
  return false;
}

bool
ExodusIO::readMeshIO()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  if(!readHeader())
    return false;
  if(!(myMesh->registerData()))
    return false;
  if(!readCoord())
    return false;
  if(!readConn())
    return false;

  int Nnp = myMesh->getNumNodes();
  if(!(myMesh->nodeOwnership = new int[Nnp]))
    return false;

  for(int i=0; i<Nnp; i++)
    myMesh->nodeOwnership[i] = 1; // all initialized to owned...

  return true;
}

bool 
ExodusIO::readHeader()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  int  err            = 0;
  int  Ndim           = 0;
  int  Nnp            = 0;
  int  Nel            = 0;
  int  Nel_tri3       = 0;
  int  Nel_truss2     = 0;
  int  Nel_quad4      = 0;
  int  Nel_quad8      = 0;
  int  Nel_hex8       = 0;
  int  Nel_hex20      = 0;
  int  Nel_hex27      = 0;
  int  Nel_tet4       = 0;
  int  Nel_tet10      = 0;
  int  num_node_sets  = 0;
  int  num_side_sets  = 0;
  int  Neblock   = 0;
  char title[MAX_LINE_LENGTH+1];
  char elemtype[MAX_STR_LENGTH+1];

  // NOTE: the number of element blocks, Neblock, is the global number.  
  // That is, every block exists on all processors, even if the block 
  // doesn't have elements on each processor.
  err = ex_get_init(myFileID, title, &Ndim, &Nnp, &Nel, &Neblock,
                    &num_node_sets, &num_side_sets);

  if (err) {
    pXcout << "ERROR: EX_get_init = << " << err << endl;
    return false;
  }

  myMesh->globalNnp = 0;
  myMesh->globalNel = 0;
  myMesh->globalNeblock = 0;
  myMesh->global_num_node_sets = 0;
  myMesh->global_num_side_sets = 0;
  err = ex_get_init_global(myFileID, &(myMesh->globalNnp), &(myMesh->globalNel),
                           &(myMesh->globalNeblock), &(myMesh->global_num_node_sets),
                           &(myMesh->global_num_side_sets));

  myMesh->setTitle(title);
  myMesh->setNumNodes( Nnp );
  myMesh->setDimensions( Ndim );

  // get global nod ids
  myMesh->nodeGlobalIds = new int[Nnp];
  this->GetExodusNodeIds(myMesh->nodeGlobalIds, myFileID);



  if( num_node_sets > 0 ) {
    int *node_set_ids = new int[num_node_sets];
    err = ex_get_ids(myFileID, EX_NODE_SET, node_set_ids);

    char** node_set_names = new char*[num_node_sets];
    for(int i=0; i<num_node_sets; i++)
       node_set_names[i] = new char[MAX_STR_LENGTH+1];

    err = ex_get_names(myFileID, EX_NODE_SET, node_set_names);
    if (err) {
      pXcout << "ERROR: ex_get_names = " << err << endl;
      return false;
    }

    int num_nodes_in_set;
    int num_dist_in_set;
    for(int i=0; i<num_node_sets; i++) {
      err = ex_get_set_param(myFileID, EX_NODE_SET, node_set_ids[i], &num_nodes_in_set,
                                  &num_dist_in_set);
      // num_dist_in_set is not used - thrown away
      myMesh->registerNodeSet(node_set_ids[i], num_nodes_in_set, node_set_names[i]);
      DMNodeSet *nsi = myMesh->getNodeSet(i);
      if(nsi->numNodes){
        int* nodes;
        myData->getVariable(nsi->NODE_LIST, nodes);
        err = ex_get_set(myFileID, EX_NODE_SET, node_set_ids[i], nodes, NULL);
        for(int j=0; j<num_nodes_in_set; j++)
          nodes[j] = nodes[j] - 1;
      }
    }
    delete [] node_set_ids;
    for(int i=0; i<num_node_sets; i++)
       delete [] node_set_names[i];
    delete [] node_set_names;
  }

  if( num_side_sets > 0 ) {
    int *side_set_ids = new int[num_side_sets];
    err = ex_get_ids(myFileID, EX_SIDE_SET, side_set_ids);

    char** side_set_names = new char*[num_side_sets];
    for(int i=0; i<num_side_sets; i++)
       side_set_names[i] = new char[MAX_STR_LENGTH+1];

    err = ex_get_names(myFileID, EX_SIDE_SET, side_set_names);
    if (err) {
      pXcout << "ERROR: ex_get_names = " << err << endl;
      return false;
    }

    int num_side_in_set;
    int num_dist_in_set;
    int num_nodes_in_set;
    for(int i=0; i<num_side_sets; i++) {
      err = ex_get_set_param(myFileID, EX_SIDE_SET, side_set_ids[i], &num_side_in_set,
                                  &num_dist_in_set);
      err = ex_get_side_set_node_list_len(myFileID, side_set_ids[i], &num_nodes_in_set);
      bool successful = myMesh->registerSideSet(side_set_ids[i], num_side_in_set, 
                                                num_nodes_in_set, side_set_names[i]);
      if(!successful){
        throw ParsingException("Fatal Error: Sideset registration failed");
      }
      DMSideSet *ssi = myMesh->getSideSet(i);
      if(ssi->numSides){
        int* sides; myData->getVariable(ssi->FACE_ID_LIST, sides);
        int* elems; myData->getVariable(ssi->ELEM_ID_LIST, elems);
        int* nodes; myData->getVariable(ssi->FACE_NODE_LIST, nodes);
        int* nodes_per_face = new int[num_side_in_set];
        err = ex_get_set(myFileID, EX_SIDE_SET, side_set_ids[i], elems, sides);
        err = ex_get_side_set_node_list(myFileID, side_set_ids[i], 
                                        nodes_per_face, nodes);

        int nnpf  = nodes_per_face[0];

          for(int iside=0; iside<num_side_in_set; iside++) {
            sides[iside] -= 1;
            elems[iside] -= 1;
            for(int inode=0; inode<nnpf; inode++)
              nodes[iside*nnpf+inode] -= 1;
          }

        delete [] nodes_per_face;
      }
    }
    delete [] side_set_ids;
    for(int i=0; i<num_side_sets; i++)
       delete [] side_set_names[i];
    delete [] side_set_names;
  }
 
  int *ids = new int[Neblock];
  int num_elem_in_block;
  int num_nodes_per_elem;
  int num_edges_per_elem;
  int num_faces_per_elem;
  int num_attr;

  err = ex_get_ids(myFileID, EX_ELEM_BLOCK, ids);
  if (err) {
    pXcout << "ERROR: EX_get_elem_blk_ids = " << err << endl;
    return false;
  }

  char** names = new char*[Neblock];
  for(int i=0; i<Neblock; i++)
     names[i] = new char[MAX_STR_LENGTH+1];

  err = ex_get_names(myFileID, EX_ELEM_BLOCK, names);
  if (err) {
    pXcout << "ERROR: ex_get_names = " << err << endl;
    return false;
  }

  // using the element block parameters read the element block info
  for (int i=0; i<Neblock; i++) {
    err = ex_get_block( myFileID, EX_ELEM_BLOCK, ids[i], elemtype,
                             &num_elem_in_block,
                             &num_nodes_per_elem, 
                             &num_edges_per_elem, 
                             &num_faces_per_elem, &num_attr);
    if (err) {
      pXcout << "ERROR: EX_get_elem_block = " << err << endl;
      return false;
    }

    if (!strncasecmp(elemtype, "NULL",   4)) {
      Topological::NullElement* eb = new Topological::NullElement( 0, 0 );
      eb->setBlockId(ids[i]); 
      eb->setBlockName(names[i]);
      myMesh->addElemBlk(eb);
    } else if (!strncasecmp(elemtype, "BEAM",   4) ||
               !strncasecmp(elemtype, "BAR2",   4)) {
//      if(Ndim == 1){
        vector<vector<int>> tFaceGraph({{0},{1}});
        Topological::Beam* eb = new Topological::Beam( num_elem_in_block, num_attr );
        Nel_truss2+=num_elem_in_block;
        eb->setDataContainer( myData );
        eb->setBlockId(ids[i]);
        eb->setBlockName(names[i]);
        eb->setFaceGraph(tFaceGraph);
        eb->registerData();
        myMesh->addElemBlk(eb);
//      }
    } else if (!strncasecmp(elemtype, "TRI",   3)) {
      if(Ndim == 2){
        vector<vector<int>> tFaceGraph({{0,1},{1,2},{2,0}});
        Topological::Tri3* eb = new Topological::Tri3( num_elem_in_block, num_attr );
        Nel_tri3+=num_elem_in_block;
        eb->setDataContainer( myData );
        eb->setBlockId(ids[i]);
        eb->setBlockName(names[i]);
        eb->setFaceGraph(tFaceGraph);
        eb->registerData();
        myMesh->addElemBlk(eb);
      }
    } else if (!strncasecmp(elemtype, "QUAD4",  5)) {
      vector<vector<int>> tFaceGraph({{0,1},{1,2},{2,3},{3,0}});
      Topological::Quad4* eb = new Topological::Quad4( num_elem_in_block, num_attr );
      Nel_quad4+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "QUAD8",  5)) {
      vector<vector<int>> tFaceGraph({{0,1,4},{1,2,5},{2,3,6},{3,0,7}});
      Topological::Quad8* eb = new Topological::Quad8( num_elem_in_block, num_attr );
      Nel_quad4+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "QUAD", 4)) {
      vector<vector<int>> tFaceGraph({{0,1},{1,2},{2,3},{3,0}});
      Topological::Quad4* eb = new Topological::Quad4( num_elem_in_block, num_attr );
      Nel_quad4+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "SHELL", 5)) {
      pXcout << "!!! Reading SHELL elements from exodus mesh file " << myName << "\n"
	     << "!!! \tSHELL elements not supported." << endl;
      return false;

    } else if (!strncasecmp(elemtype, "TETRA10", 7) ||
               !strncasecmp(elemtype, "TET10"  , 5)) {
      vector<vector<int>> tFaceGraph({{0,1,3,4,8,7},{1,2,3,5,9,8},{2,0,3,6,7,9},{0,2,1,6,5,4}});
      Topological::Tet10* eb = new Topological::Tet10( num_elem_in_block, num_attr );
      Nel_tet10+=num_elem_in_block;
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setDataContainer( myData );
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "TETRA", 5) ||
               !strncasecmp(elemtype, "TET4" , 3) ) {
      vector<vector<int>> tFaceGraph({{0,1,3},{1,2,3},{2,0,3},{0,2,1}});
      Topological::Tet4* eb = new Topological::Tet4( num_elem_in_block, num_attr );
      Nel_tet4+=num_elem_in_block;
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setDataContainer( myData );
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "HEX8",   4)) {
      vector<vector<int>> tFaceGraph({{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{3,2,1,0},{4,5,6,7}});
      Topological::Hex8* eb = new Topological::Hex8( num_elem_in_block, num_attr );
      Nel_hex8+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]); 
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "HEX20",   5)) {
      vector<vector<int>> tFaceGraph({
          {0,1,5,4, 8,13,16,12},
          {1,2,6,5, 9,14,17,13},
          {2,3,7,6,10,15,18,14},
          {3,0,4,7,11,12,19,15},
          {3,2,1,0,10, 9, 8,11},
          {4,5,6,7,16,17,18,19}});
      Topological::Hex20* eb = new Topological::Hex20( num_elem_in_block, num_attr );
      Nel_hex20+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "HEX27",   5)) {
      vector<vector<int>> tFaceGraph({
          {0,1,5,4, 8,13,16,12,25},
          {1,2,6,5, 9,14,17,13,24},
          {2,3,7,6,10,15,18,14,26},
          {3,0,4,7,11,12,19,15,23},
          {3,2,1,0,10, 9, 8,11,21},
          {4,5,6,7,16,17,18,19,22}});
      Topological::Hex27* eb = new Topological::Hex27( num_elem_in_block, num_attr );
      Nel_hex27+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else if (!strncasecmp(elemtype, "HEX",   3)) {
      vector<vector<int>> tFaceGraph({{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{3,2,1,0},{4,5,6,7}});
      Topological::Hex8* eb = new Topological::Hex8( num_elem_in_block, num_attr );
      Nel_hex8+=num_elem_in_block;
      eb->setDataContainer( myData );
      eb->setBlockId(ids[i]);
      eb->setBlockName(names[i]);
      eb->setFaceGraph(tFaceGraph);
      eb->registerData();
      myMesh->addElemBlk(eb);

    } else {
      p0cout << "!!! Reading elements from exodus mesh file " << myName << "\n"
	     << "!!! \t" << elemtype << " elements not supported." << endl;
      return false;
    }

  }

  // get element ids
  myMesh->elemGlobalIds = new int [Nel];
  this->GetExodusElementIds(myMesh->elemGlobalIds, myFileID);

  delete [] ids;

  for(int i=0; i<Neblock; i++)
     delete [] names[i];
  delete [] names;

  bool filerr = false;
  int Nel_tot = Nel_tet4 + Nel_tet10 + Nel_truss2 + Nel_tri3 + Nel_quad4 + Nel_quad8 + 
                Nel_hex8 + Nel_hex20 +Nel_hex27;
  if (Nel_tot == 0 || Nel_tot != Nel) filerr = true;
  if ( (Nel_quad4 + Nel_quad8 + Nel_tri3 + Nel_truss2 > 0) &&
       (Nel_hex8 + Nel_hex20 + Nel_hex27 + Nel_tet4 + Nel_tet10 == 0) && (Ndim == 3) ) filerr = true;
  if (filerr) {
    pXcout << "\n\n\t!!!!! Error reading exodus file !!!!!" << endl;
    pXcout << "\t  Ndim       = " << Ndim << endl;;
    pXcout << "\t  Nel_tri3   = " << Nel_tri3 << endl;
    pXcout << "\t  Nel_truss2 = " << Nel_truss2 << endl;
    pXcout << "\t  Nel_quad4  = " << Nel_quad4 << endl;
    pXcout << "\t  Nel_quad8  = " << Nel_quad8 << endl;
    pXcout << "\t  Nel_hex8   = " << Nel_hex8 << endl;
    pXcout << "\t  Nel_hex20  = " << Nel_hex20 << endl;
    pXcout << "\t  Nel_hex27  = " << Nel_hex27 << endl;
    return false;
  }

  return true;
}



bool
ExodusIO::readCoord()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  Real* x = myMesh->getX();
  Real* y = myMesh->getY();
  if( !x || !y )
    return false;
  Real* z = 0;
  if ( myMesh->getDimensions() == 3) {
    z = myMesh->getZ();
    if( !z )
      return false;
  }
  
  if (ex_get_coord(myFileID, x, y, z)) {
    pXcout << "!!! Problem getting coordinates from exodus file "
	   << myName << endl;
    return false;
  }
  return true;
}


bool
ExodusIO::writePrologue()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  if(!writeHeader())
    return false;
  if(!writeCoord())
    return false;
  if(!writeConn())
    return false;
  return true;
}

bool
ExodusIO::writeHeader()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  int  Ndim           = myMesh->getDimensions();
  int  Nnp            = myMesh->getNumNodes();
  int  Nel            = myMesh->getNumElems();
  int  num_node_sets  = myMesh->getNumNodeSets();
  int  num_side_sets  = myMesh->getNumSideSets();
  int  num_elem_blck  = myMesh->getNumElemBlks();
  char title[MAX_LINE_LENGTH+1];
  // basic info
  strcpy(title,(myMesh->getTitle())); 
  if( ex_put_init(myFileID, title, Ndim, Nnp, Nel, num_elem_blck,
		  num_node_sets, num_side_sets) ) {
    pXcout << "!!! Problem initializing exodus file "
	   << myName << endl;
    return false;
  }
  // write global node info
  ex_put_id_map(myFileID, EX_NODE_MAP, myMesh->nodeGlobalIds);
 
  // write nodeset data
  const vector<DMNodeSet>& ns = myMesh->getNodeSets();
  for(int i=0; i<num_node_sets; i++) {
    int node_set_id = ns[i].id;
    int num_nodes_in_set = ns[i].numNodes;
    int num_dist_in_set  = 0;
    if( ex_put_set_param(myFileID, EX_NODE_SET, node_set_id,num_nodes_in_set,
                              num_dist_in_set) ) {
      pXcout << "!!! Problem initializing exodus file "
             << myName << " with node set id " << node_set_id 
             << endl;
    } else {
      if(num_nodes_in_set){
        int* node_list; myData->getVariable(ns[i].NODE_LIST, node_list);
        int* mod_node_list = new int [num_nodes_in_set];
        for(int j=0; j<num_nodes_in_set; j++)
          mod_node_list[j] = node_list[j] + 1;
        if( ex_put_set(myFileID, EX_NODE_SET, node_set_id, mod_node_list, NULL)) {
          pXcout << "!!! Problem writing nodes in node set id: " << node_set_id
                 << " to file " << myName << endl;
        }
        delete [] mod_node_list;
      }
    }
  }
  // write sideset data
    vector<DMSideSet> ss = myMesh->getSideSets();
    for(int i=0; i<num_side_sets; i++) {
      int side_set_id = ss[i].id;
      int num_side_in_set = ss[i].numSides;
      int num_dist_in_set = 0;
      if( ex_put_set_param(myFileID, EX_SIDE_SET, side_set_id, num_side_in_set,
                                num_dist_in_set) ) {
        pXcout << "!!! Problem initializing exodus file "
               << myName << " with side set id " << side_set_id
               << endl;
      } else {
        if(num_side_in_set){
          int* elem_list; myData->getVariable(ss[i].ELEM_ID_LIST, elem_list);
          int* side_list; myData->getVariable(ss[i].FACE_ID_LIST, side_list);
          int* tmpelem = new int [num_side_in_set];
          int* tmpside = new int [num_side_in_set];
          for(int iside=0; iside<num_side_in_set; iside++) {
            tmpside[iside] = side_list[iside] + 1;
            tmpelem[iside] = elem_list[iside] + 1;
          }
          if( ex_put_set(myFileID, EX_SIDE_SET, side_set_id, tmpelem, tmpside)) {
            pXcout << "!!! Problem writing sides in side set id: " << side_set_id
                   << " to file " << myName << endl;
          }
          delete [] tmpside;
          delete [] tmpelem;
        }
      }
    }

    ex_put_init_global(myFileID, myMesh->globalNnp, myMesh->globalNel,
                             myMesh->globalNeblock, myMesh->global_num_node_sets,
                             myMesh->global_num_side_sets);

    ex_put_map_param(myFileID, 1, 1);

    ex_put_id_map(myFileID, EX_NODE_MAP, myMesh->nodeGlobalIds);
    ex_put_id_map(myFileID, EX_ELEM_MAP, myMesh->elemGlobalIds);

  return true;
}

bool
ExodusIO::writeCoord()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  Real* x = myMesh->getX();
  Real* y = myMesh->getY();
  Real* z = NULL;
  if( myMesh->getDimensions() == 3 )
    z = myMesh->getZ();
  if(ex_put_coord(myFileID, x, y, z)) {
    pXcout << "!!! Problem writing coords to exodus file "
	   << myName << endl;
    return false;
  }
  return true;
}

bool
ExodusIO::readConn()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  

  char elemtype[MAX_STR_LENGTH+1];
  int  Neblock   = myMesh->getNumElemBlks();  
  int* num_attr = new int[Neblock];
  
  // assume that the node and element maps are sequential,
  // i.e. there is no alternative numbering implied
  // and go straight to element block information

  int* num_elem_in_block = new int[Neblock];
  int* num_nodes_per_elem = new int[Neblock];
  int* num_edges_per_elem = new int[Neblock];
  int* num_faces_per_elem = new int[Neblock];
  
  int global_element_count = 0;
  for( int i=0; i<Neblock; i++ ) {
    Topological::Element* elblock = myMesh->getElemBlk(i);
    int block_id = elblock->getBlockId();
    if(elblock->getNumElem() == 0) continue;
    if(ex_get_block( myFileID, EX_ELEM_BLOCK,  
			  block_id,
			  elemtype,
			  &(num_elem_in_block[i]),
			  &(num_nodes_per_elem[i]),
			  &(num_edges_per_elem[i]),
			  &(num_faces_per_elem[i]),
			  &(num_attr[i]))) {
      pXcout << "!!! Problem reading element block " << block_id << "\n"
	     << "!!! \tfrom exodus file "
	     << myName << endl;
      delete [] num_elem_in_block;
      delete [] num_nodes_per_elem;
      delete [] num_edges_per_elem;
      delete [] num_faces_per_elem;
      delete [] num_attr;
      return false;
    }
    

    int tmpnn = num_elem_in_block[i]*num_nodes_per_elem[i];
    int *connect = new int[tmpnn];

    if(ex_get_conn (myFileID, EX_ELEM_BLOCK, block_id, connect, 0, 0)) {
      pXcout << "ERROR: EX_get_elem_conn = " << endl;
      return false;
    }

    // we store global node numbers internally as 0-based
    // .... while exodus is 1-based -  
    for(int itmp=0; itmp<tmpnn; itmp++)
      connect[itmp] = connect[itmp]-1;

    // instantiate/build base element at this point...
    Topological::Element* eb = myMesh->getElemBlk(i);
    if (!strncasecmp(elemtype, "QUAD4",  5)) {
      const int NNPE = 4;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "QUAD8",  5)) {
      const int NNPE = 8;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "QUAD",  4)) {
      const int NNPE = 4;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
        eb->connectNodes(k, global_element_count, loconn);
        loconn+=NNPE;
        ++global_element_count;
      }
    } else if(!strncasecmp(elemtype, "HEX8", 4)) {
      const int NNPE = 8;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if(!strncasecmp(elemtype, "HEX20", 20)) {
      const int NNPE = 20;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if(!strncasecmp(elemtype, "HEX27", 27)) {
      const int NNPE = 27;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if(!strncasecmp(elemtype, "HEX", 3)) {
      const int NNPE = 8;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
        eb->connectNodes(k, global_element_count, loconn);
        loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "TRUSS",  5)) {
      const int NNPE = 2;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "BEAM",  4) ||
               !strncasecmp(elemtype, "BAR2",  4)) {
      const int NNPE = 2;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
        eb->connectNodes(k, global_element_count, loconn);
        loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "TRI",  3)) {
      const int NNPE = 3;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "TETRA10", 7) ||
               !strncasecmp(elemtype, "TET10",  5) ) {
      const int NNPE = 10;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } else if (!strncasecmp(elemtype, "TETRA", 5) ||
               !strncasecmp(elemtype, "TET4",  3) ) {
      const int NNPE = 4;
      int* loconn = connect;
      for( int k=0; k<num_elem_in_block[i]; ++k ) {
	eb->connectNodes(k, global_element_count, loconn);
	loconn+=NNPE;
        ++global_element_count;
      }
    } 
    delete [] connect;

    if(num_attr[i]){
      // get pointer to elem attributes from the topoelement, eb
      double* attributes = eb->getAttributes();
      if(ex_get_attr (myFileID, EX_ELEM_BLOCK, block_id, attributes)) {
        p0cout << "ERROR: EX_get_elem_conn = " << endl;
        return false;
      }
    }
  }

//  delete [] ids;
  delete [] num_elem_in_block;
  delete [] num_nodes_per_elem;
  delete [] num_edges_per_elem;
  delete [] num_faces_per_elem;
  delete [] num_attr;

  return true;
}

bool
ExodusIO::writeConn()
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  int Neblk = myMesh->getNumElemBlks();
  for( int i=0; i<Neblk; i++ ) {

    Topological::Element* elblock = myMesh->getElemBlk(i);
    int  Nel     = elblock->getNumElem();

    auto type = elblock->getType();
    int  Nnpe    = elblock->getNnpe();
    int  tmpnn   = Nnpe*Nel;
    int* connect = elblock->getNodeConnect();
    int ebid = elblock->getBlockId();
    int Nattr = 0;
    if( ex_put_block( myFileID, EX_ELEM_BLOCK, ebid, type.c_str(), Nel, Nnpe, 0, 0, Nattr ) ) {
      pXcout << "!!! Problem writing element block " << i+1 << "\n"
	     << "!!! \tto exodus file "
	     << myName << endl;
      return false;

    }

    // if there are no elements from this block in this proc, don't write conn.
    if(Nel == 0) continue;
  
    int* tmpconn = new int [tmpnn];
    for(int n=0;n<tmpnn;n++) tmpconn[n] = connect[n] + 1;  // exodus numbers from one
    if( ex_put_conn( myFileID, EX_ELEM_BLOCK, ebid, tmpconn, 0, 0) ) {
      pXcout << "!!! Problem writing connectivity for element block " << i << "\n"
             << "!!! \tto exodus file "
             << myName << endl;
      delete [] tmpconn;
      return false;
    }
    delete [] tmpconn;
  }
  return true;
}

bool
ExodusIO::initVars(DataCentering centering, 
                   int num_vars,
                   vector<string> names )
{
#ifdef DEBUG_LOCATION
  _print_entering_location(__AXSIS_FUNCTION_NAMER__);
#endif //DEBUG_LOCATION
  
  bool status = true;
  char** vname = new char* [num_vars];
  for(int i=0; i<num_vars; i++) 
    vname[i] = (char*) names[i].c_str();

  switch( centering ) {
    case NODE:
    {
      if( ex_put_variable_param(myFileID, EX_NODAL, num_vars) ) {
        status = false;
        pXcout << "!!! Problem writing number of plot node variables " << "\n"
  	       << "!!! \tto exodus file "
	       << myName << endl;
        break;
      }
      if( ex_put_variable_names(myFileID, EX_NODAL, num_vars, vname) ) {
        status = false;
        pXcout << "!!! Problem writing plot element variable names " << "\n"
  	       << "!!! \tto exodus file "
	       << myName << endl;
        break;
      }
      break;
    }
    case ELEM:
    {
      if( ex_put_variable_param(myFileID, EX_ELEM_BLOCK, num_vars) ) {
        status = false;
        pXcout << "!!! Problem writing number of plot element variables " << "\n"
  	       << "!!! \tto exodus file "
	       << myName << endl;
        break;
      }
      if( ex_put_variable_names(myFileID, EX_ELEM_BLOCK, num_vars, vname) ) {
        status = false;
        pXcout << "!!! Problem writing plot element variable names" << "\n"
  	       << "!!! \tto exodus file "
	       << myName << endl;
        break;
      }
      break;
    }
    default:
      status = false;
      pXcout << "!!! Unsupported variable centering [" << centering << "] for ExodusIO.\n"
             << "!!! \tOutput attempted to exodus file "
             << myName << endl;
      break;
  }
  delete [] vname;
  return status;
}

bool
ExodusIO::closeMeshIO()
{
  // If output file was never opened, don't close it
  if( myFileID == -1 ) return true;
  
  int errCode = ex_close(myFileID);
  if( errCode ) {
    pXcout << "!!! Problem closing exodus mesh file " << myName << "\n"
	   << endl;
    return false;
  }

  return true;
}

bool
ExodusIO::writeTitle()
{
  return true;  
}

bool
ExodusIO::writeQA()
{
  return true;  
}

int
ExodusIO::testFile(const char *file_name)
{
  int err;
  ifstream test_file;

  test_file.open(file_name);
  if (test_file.bad()) {
    err = -1;
  } else {
    test_file.close();
    err = 0;
  }
  return err;
}

/******************************************************************************//**
* \brief Begin writing new time plane.
* \param [in] aTimeStep Zero-based index into time steps array
* \param [in] aTimeValue Time value.  Value must be greater than the previous time value.
**********************************************************************************/
bool 
ExodusIO::writeTime(int aTimeStep, Real aTimeValue)
{
  auto tOneBasedTimeStep = aTimeStep + 1;
  ex_put_time(myFileID, tOneBasedTimeStep, &aTimeValue);
  return true;
}

/******************************************************************************//**
* \brief Get the number of time steps stored in the exodus database
**********************************************************************************/
int
ExodusIO::getNumSteps()
{
  return ex_inquire_int(myFileID, EX_INQ_TIME);
}

/******************************************************************************//**
* \brief Get nodal variable names from the exodus database
**********************************************************************************/
std::vector<std::string>
ExodusIO::getNodeVarNames()
{
  std::vector<std::string> tReturnNames;

  int tNumNodeVars;
  ex_get_variable_param(myFileID, EX_NODAL, &tNumNodeVars);

  char** tNames = new char*[tNumNodeVars];
  for(int i=0; i<tNumNodeVars; i++)
    tNames[i] = new char[MAX_STR_LENGTH+1];

  ex_get_variable_names(myFileID, EX_NODAL, tNumNodeVars, tNames);

  for(int i=0; i<tNumNodeVars; i++)
  {
    std::string tVarName(tNames[i]);
    tReturnNames.push_back(tVarName);
    delete [] tNames[i];
  }
  delete [] tNames;

  return tReturnNames;
}

/******************************************************************************//**
* \brief Write node plot to exodus mesh
* \param [in] aData pointer to node data of length numNodes
* \param [in] aVariableIndex zero-based index into nodal variables array
* \param [in] aStepIndex zero-based index into time step array
**********************************************************************************/
bool
ExodusIO::writeNodePlot(Real* aData, int aVariableIndex, int aStepIndex)
{
  int tNumNodes = myMesh->getNumNodes();

  // index arguments to this function are zero-based.  The exodus API is one-based:
  auto tOneBasedVariableIndex = aVariableIndex+1;
  auto tOneBasedStepIndex = aStepIndex+1;
  ex_put_var(myFileID, tOneBasedStepIndex, EX_NODAL, tOneBasedVariableIndex, /*obj_id=*/1, tNumNodes, aData);
  ex_update(myFileID);
  return true;
}

/******************************************************************************//**
* \brief Read node plot from exodus mesh
* \param [out] aData Pointer to return data
* \param [in] aVariableName Name of nodal variable to read
* \param [in] aStepIndex zero-based index into time step array
**********************************************************************************/
bool
ExodusIO::readNodePlot(double* aData, string aVariableName, int aStepIndex)
{

  int tNumTimeSteps;
  float tDummyFloat;
  char tDummyChar;
  ex_inquire(myFileID, EX_INQ_TIME, &tNumTimeSteps, &tDummyFloat, &tDummyChar);

  int tReadTimeStep = -1;
  int tOneBasedStepIndex = aStepIndex+1;
  if( tOneBasedStepIndex == 0 ) {
    tReadTimeStep = tNumTimeSteps;
  } else {
    if( tOneBasedStepIndex > tNumTimeSteps || tOneBasedStepIndex < 1 )
    {
      throw ParsingException("Fatal Error: Requested time step doesn't exist");
    }
    tReadTimeStep = tOneBasedStepIndex;
  }

  int tNumNodeVars;
  ex_get_variable_param(myFileID, EX_NODAL, &tNumNodeVars);

  char** tNames = new char*[tNumNodeVars];
  for(int i=0; i<tNumNodeVars; i++)
     tNames[i] = new char[MAX_STR_LENGTH+1];
  ex_get_variable_names(myFileID, EX_NODAL, tNumNodeVars, tNames);

  int tOneBasedVarIndex=-1;
  for(int i=0; i<tNumNodeVars; i++)
    if(!strncasecmp(tNames[i],aVariableName.c_str(),aVariableName.length())) {
      tOneBasedVarIndex=i+1;
      break;
    }
 
  if (tOneBasedVarIndex < 1)
  {
      std::stringstream tMessage;
      tMessage << "Fatal Error: Requested variable that doesn't exist" << std::endl;
      tMessage << "Requested: " << aVariableName << std::endl;
      tMessage << "Available: " << std::endl;
      for(int iName=0; iName<tNumNodeVars; iName++)
      {
          tMessage << "  " << tNames[iName] << std::endl;
      }
      throw ParsingException(tMessage.str());
  }

  for(int i=0; i<tNumNodeVars; i++)
     delete [] tNames[i];
  delete [] tNames;

  int tNumNodes = myMesh->getNumNodes();
  ex_get_var(myFileID, tReadTimeStep, EX_NODAL, tOneBasedVarIndex, 1, tNumNodes, aData);

  return true;
}

/******************************************************************************//**
* \brief Write element plot to exodus mesh
* \param [in] aData pointer to element data of length numElements
* \param [in] aVariableIndex zero-based index into element variables array
* \param [in] aStepIndex zero-based index into time step array
**********************************************************************************/
bool
ExodusIO::writeElemPlot(Real* aData, int aVariableIndex, int aStepIndex)
{
  // index arguments to this function are zero-based.  The exodus API is one-based:
  int tOneBasedVariableIndex = aVariableIndex+1;
  int tOneBasedStepIndex = aStepIndex+1;

  int tNumElemBlocks = myMesh->getNumElemBlks();
  Real *tOffsetData = NULL;
  int tElemCount = 0;
  for(int iBlock=0; iBlock<tNumElemBlocks; iBlock++) {
    int tNumElemInBlock = myMesh->getNumElemInBlk(iBlock);
    if(tNumElemInBlock == 0) continue;
    int tElemBlockID = myMesh->getBlockId(iBlock);
    tOffsetData = &aData[tElemCount];
    ex_put_var(myFileID, tOneBasedStepIndex, EX_ELEM_BLOCK, tOneBasedVariableIndex, tElemBlockID,
                    tNumElemInBlock, tOffsetData);
    tElemCount += tNumElemInBlock; //assumes all blocks have same data
  }
  ex_update(myFileID);
  return true;
}

bool
ExodusIO::readElemPlot(double* data, string name)
{

  int num_time_steps;
  float fdum;
  char cdum;
  ex_inquire(myFileID, EX_INQ_TIME, &num_time_steps, &fdum, &cdum);

  assert(num_time_steps == 1);

  int num_elem_vars;
  ex_get_variable_param(myFileID, EX_ELEM_BLOCK, &num_elem_vars);

  char** names = new char*[num_elem_vars];
  for(int i=0; i<num_elem_vars; i++)
     names[i] = new char[MAX_STR_LENGTH+1];
  ex_get_variable_names(myFileID, EX_ELEM_BLOCK, num_elem_vars, names);

  int varindex=-1;
  for(int i=0; i<num_elem_vars; i++)
    if(!strncasecmp(names[i],name.c_str(),name.length())) {
      varindex=i+1;
      break;
    }
 
  if (varindex < 0) {
   p0cout << "ERROR:  Input variable " << name << " not found." << endl;
   return false;
  }

  int num_elem_blocks = myMesh->getNumElemBlks();
  Real *offset_data = NULL;
  int elem_count = 0;
  for(int i=0; i<num_elem_blocks; i++) {
    int num_elem_in_block = myMesh->getNumElemInBlk(i);
    if(num_elem_in_block == 0) continue;
    int ebid = myMesh->getBlockId(i);
    offset_data = &data[elem_count];
    ex_get_var(myFileID, num_time_steps, EX_ELEM_BLOCK, varindex, ebid, 
                    num_elem_in_block, offset_data);
    elem_count += num_elem_in_block; //assumes all blocks have same data
  }


  return true;
}
// --------------------------------------------------------------------------------------------------------
void ExodusIO::GetExodusNodeIds(int * a_NodeIds, int a_MyFileId)
{
   bool tIsParallel = (WorldComm.GetSize() > 1);

   if(tIsParallel)
   {
      GetParallelNodeIds(a_NodeIds, a_MyFileId);
   }
   else
   {
      GetSerialNodeIds(a_NodeIds, a_MyFileId);
   }
}

void ExodusIO::GetSerialNodeIds(int * a_NodeIds, int a_MyFileId)
{
    if(myIgnoreNodeMap){
        int Nnp = myMesh->getNumNodes();
        for( int iNode=0; iNode<Nnp; iNode++ ){
            a_NodeIds[iNode] = iNode+1;
        }
    } else {
        int map_count = ex_inquire_int(a_MyFileId, EX_INQ_NODE_MAP);
        if(map_count == 1)
        {
            ex_get_num_map(a_MyFileId, EX_NODE_MAP, 1, a_NodeIds);
        }
        else
        {
            ex_get_id_map(a_MyFileId, EX_NODE_MAP, a_NodeIds);
        }
    }
}

void ExodusIO::GetParallelNodeIds(int * a_NodeIds, int a_MyFileId)
{
   int map_count = 0;
   if(!myIgnoreNodeMap)
   {
      map_count = ex_inquire_int(a_MyFileId, EX_INQ_NODE_MAP);
   }

   if(map_count == 1)
   {
       ex_get_num_map(a_MyFileId, EX_NODE_MAP, 1, a_NodeIds);
   }
   else
   {
       ex_get_id_map(a_MyFileId, EX_NODE_MAP, a_NodeIds);
   }
}
// ----------------------------------------------------------------------------------------------------------
void ExodusIO::GetExodusElementIds(int * a_ElemIds, int a_MyFileId)
{
   bool tIsParallel = (WorldComm.GetSize() > 1);

   if(tIsParallel)
   {
      GetParallelElementIds(a_ElemIds, a_MyFileId);
   }
   else
   {
      GetSerialElementIds(a_ElemIds, a_MyFileId);
   }
}

void ExodusIO::GetSerialElementIds(int * a_ElemIds, int a_MyFileId)
{
    if(myIgnoreElemMap)
    {
        int Nne = myMesh->getNumElems();
        for( int iElem=0; iElem<Nne; iElem++ ){
            a_ElemIds[iElem] = iElem+1;
        }
    } else {
        int map_count = ex_inquire_int(a_MyFileId, EX_INQ_ELEM_MAP);
        if(map_count == 1)
        {
            ex_get_num_map(a_MyFileId, EX_ELEM_MAP, 1, a_ElemIds);
        }
        else
        {
            ex_get_id_map(a_MyFileId, EX_ELEM_MAP, a_ElemIds);
        }
    }
}

void ExodusIO::GetParallelElementIds(int * a_ElemIds, int a_MyFileId)
{
   int map_count = 0;
   if(!myIgnoreElemMap)
   {
      map_count = ex_inquire_int(a_MyFileId, EX_INQ_ELEM_MAP);
   }

   if(map_count == 1)
   {
       ex_get_num_map(a_MyFileId, EX_ELEM_MAP, 1, a_ElemIds);
   }
   else
   {
       ex_get_id_map(a_MyFileId, EX_ELEM_MAP, a_ElemIds);
   }
}
