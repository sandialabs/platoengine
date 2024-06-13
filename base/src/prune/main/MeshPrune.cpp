/*
 * MeshPrune.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: tzirkle
 */
#include "MeshPrune.hpp"

#include "PruneMeshAPISTK.hpp"

void MeshPrune::prune_mesh(MeshManager &tMeshManager)
{
    tMeshManager.define_iso_fields();
    prune::PerceptPrune pruner;
    const std::string tFieldName{"LSD"};
    const std::string tOutputFieldString{""};
    constexpr double tMinEdgeLength = 0.0;
    constexpr double tIsoValue = 0.0;
    constexpr bool tConcatenateResults = false;
    constexpr bool tOnlyCreateIsoTriangles = true;
    constexpr bool tReadSpreadFile = 0;
    const int tNumberOfBufferLayers = tMeshManager.get_buffer_layers();
    const bool tAllowNonmanifoldConnections = tMeshManager.allow_nonmanifold_connections();
    if(pruner.import(tFieldName,tOutputFieldString,tMinEdgeLength,
                     tIsoValue,tConcatenateResults,tOnlyCreateIsoTriangles,tReadSpreadFile,
                     tAllowNonmanifoldConnections,tNumberOfBufferLayers))
    {
        pruner.run_percept_mesh_stand_alone(tMeshManager);
    }
}


