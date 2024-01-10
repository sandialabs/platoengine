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

#include "data_container.hpp"
#include "exception_handling.hpp"
#include "types.hpp"
#include <math.h>
#include <Teuchos_ParameterList.hpp>
#include <Teuchos_XMLParameterListHelpers.hpp>


#ifdef DEBUG
#include "communicator.hpp"
#endif

#include "topological_element.hpp"

namespace Topological {
std::string strint(const std::string s, const int i) {
   std::ostringstream ss;
    ss << s << "_" << i;
    return ss.str();
  }
}

using namespace Topological;

/*****************************************************************************/
ElementIntegration::~ElementIntegration()
/*****************************************************************************/
{
  if(cubPoints) delete cubPoints;
  if(cubWeights) delete cubWeights;
}

/*****************************************************************************/
IntrepidIntegration::IntrepidIntegration(pugi::xml_node& node, 
                                         Teuchos::RCP<shards::CellTopology> blockTopology )
/*****************************************************************************/
{
  int integrationOrder = Plato::Parse::getInt( node, "order" );
  Intrepid::DefaultCubatureFactory<double> cubfactory;
  cubature = cubfactory.create( *blockTopology, integrationOrder );
  int dim = cubature->getDimension();
  int numPoints = cubature->getNumPoints();
  cubPoints = new Intrepid::FieldContainer<double>(numPoints, dim);
  cubWeights = new Intrepid::FieldContainer<double>(numPoints);
  cubature->getCubature(*cubPoints, *cubWeights);
}

/*****************************************************************************/
CustomIntegration::CustomIntegration(pugi::xml_node& node, int myDim )
/*****************************************************************************/
{
  int numPoints = Plato::Parse::numChildren(node, "weights" );
  int xpoints = Plato::Parse::numChildren(node, "x" );
  int ypoints = Plato::Parse::numChildren(node, "y" );
  if( myDim == 3 ){
    int zpoints = Plato::Parse::numChildren(node, "z" );
    if( xpoints != ypoints || xpoints != zpoints || xpoints != numPoints )
      throw ParsingException("custom integration points: !(len(x) == len(y) == len(z) == len(weights))" );
  } else if( myDim == 2 ) {
    if( xpoints != ypoints || xpoints != numPoints )
      throw ParsingException("custom integration points: !(len(x) == len(y) == len(weights))" );
  }

  cubPoints = new Intrepid::FieldContainer<double>(numPoints, myDim);
  cubWeights = new Intrepid::FieldContainer<double>(numPoints);

  Intrepid::FieldContainer<double>& p = *cubPoints;
  Intrepid::FieldContainer<double>& w = *cubWeights;
    
  pugi::xml_node cur_weight = node.child("weights");
  pugi::xml_node cur_x = node.child("x");
  pugi::xml_node cur_y = node.child("y");
  pugi::xml_node cur_z = node.child("z");
  for( int ip=0; ip<numPoints; ip++){
    w(ip) = Plato::Parse::getDouble(cur_weight, "weight" );
    p(0,ip) = Plato::Parse::getDouble(cur_x, "x" );
    p(1,ip) = Plato::Parse::getDouble(cur_y, "y" );
    if( myDim == 3 )
    {
      p(2,ip) = Plato::Parse::getDouble(cur_z, "z" );
      cur_z = cur_z.child("z");
    }
    cur_weight = cur_weight.child("weights");
    cur_x = cur_x.child("x");
    cur_y = cur_y.child("y");
  }
}

/*****************************************************************************/
Element::~Element()
/*****************************************************************************/
{
  if(elementIntegration) delete elementIntegration;
  if(blockBasis) delete blockBasis;
}

/*****************************************************************************/
void Element::setIntegrationMethod(pugi::xml_node& node)
/*****************************************************************************/
{
  // don't try to create an integration rule if the element is empty
  if (myNel == 0) return;

  string intgType = Plato::Parse::getString(node, "type");
  if( intgType == "gauss" ){
    elementIntegration = new IntrepidIntegration( node, blockTopology );
  } 
  else
  if( intgType == "custom" ){
    elementIntegration = new CustomIntegration( node, myDim );
  } 
}

/*****************************************************************************/
void Element::Connect(int* gid, int lid)
/*****************************************************************************/
{
  int* top = nodeConnect + lid*myNnpe;
  for(int i=0;i<myNnpe;i++) gid[i] = top[i];
}

void Element::setDataContainer(DataContainer* dc) { myData = dc; }
int* Element::Connect(int lid) { return nodeConnect + lid*myNnpe; }
int* Element::getNodeConnect() { return( nodeConnect ); }

/*****************************************************************************/
void Element::connectNodes( int lid, int gid, int* node_list )
/*****************************************************************************/
{
#ifdef DEBUG
  if( lid>myNel ) {
    p0cout << "!!! Problem connecting nodes to element" << endl;
  }
#endif
  int element_offset = lid*myNnpe;
  globalID[lid] = gid;
  for( int i=0; i<myNnpe; i++ )
    nodeConnect[element_offset+i] = node_list[i];
}

/*****************************************************************************/
Element::Element( int number, int nattr ){ 
/*****************************************************************************/
  zeroset();
  myNattr = nattr;
  myNel  = number;
}


void Tri3::init()
{
  myType = "TRI3";
  myData = NULL;
  myNnpe = 3;
  myNnps = 2;
  myDim = 2;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
  blockTopology = Teuchos::rcp(new shards::CellTopology(shards::getCellTopologyData<shards::Triangle<3> >() ) );
  blockBasis = new Intrepid::Basis_HGRAD_TRI_C1_FEM<double, Intrepid::FieldContainer<double> >() ;
}


Tri3::~Tri3()
{
}


void
Tri3::registerData()
{
  int number = myNel*myNnpe;
  
  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONNT3",groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("T3GID",groupID),
					  ELEM,
					  myNel );

  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                          strint("T3ATR",groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }
  
  myData->getVariable( NODECONNECT, nodeConnect );
  myData->getVariable( GLOBALID, globalID );
  
}

Beam::~Beam()
{
}

void Beam::registerData()
{
  int number = myNel*myNnpe;

  NODECONNECT = myData->registerVariable( IntType,
                                          strint("CONNB2",groupID),
                                          UNSET,
                                          number );

  GLOBALID    = myData->registerVariable( IntType,
                                          strint("B2GID",groupID),
                                          ELEM,
                                          myNel );

  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType,
                                           strint("B2ATR",groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }

  myData->getVariable( NODECONNECT, nodeConnect );
  myData->getVariable( GLOBALID, globalID );

}

void Beam::CurrentCoordinates(int* node_gid_list, Real** X, Real* curcoor)
{
  Real*& x = X[0];
  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
  }
}

void Beam::init()
{
  myType = "BEAM";
  myData = NULL;
  myNnpe = 2;
  myNnps = 1;
  myDim = 1;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
  blockTopology = Teuchos::rcp(new shards::CellTopology(shards::getCellTopologyData<shards::Line<2> >() ) );
  blockBasis = new Intrepid::Basis_HGRAD_LINE_C1_FEM<double, Intrepid::FieldContainer<double> >() ;
}

void Tri3::CurrentCoordinates(int* node_gid_list, Real** X, Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+3] = y[node_gid_list[i]];
  }
}


void Quad8::init()
{
  myType = "QUAD8";
  myData = NULL;
  myNnpe = 8;
  myNnps = 2;
  myDim = 2;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
}

Quad8::~Quad8()
{
}

void
Quad8::registerData()
{
  int number = myNel*myNnpe;
  
  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONNQ8", groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("Q8GID", groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                          strint("Q8ATR", groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }
  
  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);
}

void Quad8::CurrentCoordinates(int* node_gid_list, Real** X, Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+8] = y[node_gid_list[i]];
  }


}



void Quad4::init()
{
  myType = "QUAD4";
  myData = NULL;
  myNnpe = 4;
  myNnps = 2;
  myDim = 2;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
}
 
Quad4::~Quad4()
{
}

void
Quad4::registerData()
{
  int number = myNel*myNnpe;
  
  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONNQ4", groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("Q4GID", groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                          strint("Q4ATR", groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }
  
  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);
  
}



void Quad4::CurrentCoordinates(int* node_gid_list, Real** X, Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+4] = y[node_gid_list[i]];
  }


}

void Hex8::init()
{
  myType = "HEX8";
  myData = NULL;
  myNnpe = 8;
  myNnps = 4;
  myDim = 3;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
  blockTopology = Teuchos::rcp(new shards::CellTopology(shards::getCellTopologyData<shards::Hexahedron<8> >() ) );
  blockBasis = new Intrepid::Basis_HGRAD_HEX_C1_FEM<double, Intrepid::FieldContainer<double> >() ;
}

Hex8::~Hex8()
{
}

void Hex8::CurrentCoordinates(int* node_gid_list, 
                              Real** X,
                              Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  Real*& z = X[2];

  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+8] = y[node_gid_list[i]];
    curcoor[i+16]= z[node_gid_list[i]];
  }
}


void
Hex8::registerData()
{
  int number = myNel*myNnpe;


  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONNH8",groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("H8GID",groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                          string("H8ATR", groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }
  
  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);

}


void Hex20::init()
{
  myType = "HEX20";
  myData = NULL;
  myNnpe = 20;
  myNnps = 8;
  myDim = 3;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
}

Hex20::~Hex20()
{
}

void Hex20::CurrentCoordinates(int* node_gid_list, 
                              Real** X,
                              Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  Real*& z = X[2];

  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+20]= y[node_gid_list[i]];
    curcoor[i+40]= z[node_gid_list[i]];
  }
}


void
Hex20::registerData()
{
  int number = myNel*myNnpe;

  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONNH20", groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("H20GID", groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                          strint("H20ATR", groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }
  
  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);

}

void Hex27::init()
{
  myType = "HEX27";
  myData = NULL;
  myNnpe = 27;
  myNnps = 9;
  myDim = 3;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
  blockTopology = Teuchos::rcp(new shards::CellTopology(shards::getCellTopologyData<shards::Hexahedron<27> >() ) );
  blockBasis = new Intrepid::Basis_HGRAD_HEX_C2_FEM<double, Intrepid::FieldContainer<double> >() ;
}

Hex27::~Hex27()
{
}

void Hex27::CurrentCoordinates(int* node_gid_list, 
                              Real** X,
                              Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  Real*& z = X[2];

  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+27]= y[node_gid_list[i]];
    curcoor[i+54]= z[node_gid_list[i]];
  }
}


void
Hex27::registerData()
{
  int number = myNel*myNnpe;

  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONNH27", groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("H27GID", groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                          strint("H27ATR", groupID),
                                          UNSET,
                                          number );
    myData->getVariable(ATTRIBUTES, attributes);
  }
  
  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);

}




void Tet4::init()
{
  myType = "TET4";
  myData = NULL;
  myNnpe = 4;
  myNnps = 3;
  myDim = 3;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
  blockTopology = Teuchos::rcp(new shards::CellTopology(shards::getCellTopologyData<shards::Tetrahedron<4> >() ) );
  blockBasis = new Intrepid::Basis_HGRAD_TET_C1_FEM<double, Intrepid::FieldContainer<double> >() ;
}

Tet4::~Tet4()
{
}

void Tet4::CurrentCoordinates(int* node_gid_list, 
                              Real** X,
                              Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  Real*& z = X[2];

  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+4] = y[node_gid_list[i]];
    curcoor[i+8] = z[node_gid_list[i]];
  }
}


void
Tet4::registerData()
{
  int number = myNel*myNnpe;

  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONT4", groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("T4GID", groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                           strint("T4ATR", groupID),
                                           UNSET,
                                           number );
    myData->getVariable(ATTRIBUTES, attributes);
  }

  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);

}

void Tet10::init()
{
  myType = "TETRA10";
  myData = NULL;
  myNnpe = 10;
  myNnps = 6;
  myDim = 3;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;
  blockTopology = Teuchos::rcp(new shards::CellTopology(shards::getCellTopologyData<shards::Tetrahedron<10> >() ) );
  blockBasis = new Intrepid::Basis_HGRAD_TET_C2_FEM<double, Intrepid::FieldContainer<double> >() ;
}

Tet10::~Tet10()
{
}

void Tet10::CurrentCoordinates(int* node_gid_list, 
                              Real** X,
                              Real* curcoor)
{
  Real*& x = X[0];
  Real*& y = X[1];
  Real*& z = X[2];

  for(int i=0;i<myNnpe;i++){
    curcoor[i]   = x[node_gid_list[i]];
    curcoor[i+10] = y[node_gid_list[i]];
    curcoor[i+20] = z[node_gid_list[i]];
  }
}


void
Tet10::registerData()
{
  int number = myNel*myNnpe;

  NODECONNECT = myData->registerVariable( IntType,
					  strint("CONT10", groupID),
					  UNSET,
					  number );

  GLOBALID    = myData->registerVariable( IntType,
					  strint("T10GID", groupID),
					  ELEM,
					  myNel );
  
  number = myNel*myNattr;
  if(myNattr){
    ATTRIBUTES = myData->registerVariable( RealType, 
                                           strint("T10ATR", groupID),
                                           UNSET,
                                           number );
    myData->getVariable(ATTRIBUTES, attributes);
  }

  myData->getVariable(NODECONNECT, nodeConnect);
  myData->getVariable(GLOBALID, globalID);

}
void Element::zeroset()
{
  myData = NULL;
  myNnpe = 0;
  myNnps = 0;
  myDim = 0;
  myNel  = 0;
  myNattr = 0;
  NODECONNECT = UNSET_VAR_INDEX;
  GLOBALID    = UNSET_VAR_INDEX;

  elementIntegration = nullptr;
  blockBasis = nullptr;
}

void NullElement::CurrentCoordinates(int* /*node_gid_list*/, Real** /*X*/,
                              Real* /*curcoor*/) { assert(-1); }

void
NullElement::registerData()
{
  nodeConnect = NULL;
  globalID    = NULL;
}


