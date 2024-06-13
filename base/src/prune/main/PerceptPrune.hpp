/*
 * PerceptPrune.hpp
 *
 *  Created on: Aug 1, 2017
 *      Author: tzirkle
 */

#ifndef ISO_PRUNE_MAIN_PERCEPTPRUNE_HPP_
#define ISO_PRUNE_MAIN_PERCEPTPRUNE_HPP_

#include <sstream>
#include <vector>
#include <iostream>

#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/CoordinateSystems.hpp>

#include "Teuchos_CommandLineProcessor.hpp"
#include "PruneMeshAPISTK.hpp"

#include <percept/xfer/STKMeshTransferSetup.hpp>
#include <percept/PerceptMesh.hpp>
#include <percept/function/StringFunction.hpp>
#include <percept/function/FieldFunction.hpp>
#include <percept/function/ConstantFunction.hpp>
#include <percept/Percept.hpp>
#include <percept/Util.hpp>
#include <percept/ExceptionWatch.hpp>
#include <percept/GeometryVerifier.hpp>

#include "MeshManager.hpp"

namespace stk
{

namespace io { class StkMeshIoBroker; }

}

namespace prune
{

class PerceptPrune
{
public:
    ~PerceptPrune();
    bool import(const std::string &aFieldName,
                const std::string &aOutputFieldsString,
                const double &aMinEdgeLength,
                const double &aIsoValue,
                const bool aConcatenateResults,
                const bool aOnlyCreateIsoTriangles,
                const bool aReadSpreadFile,
                const bool aAllowNonmanifoldConnections,
                const int aNumberOfBufferLayers);
    bool run_percept_mesh_stand_alone(MeshManager & aMeshManager);

private:
    bool run_percept_mesh_private_stand_alone(MeshManager & aMeshManager);

    std::string mMeshIn{""};
    std::string mMeshOut{""};
    std::string mFieldName{""};
    std::string mOutputFieldsString{""};
    std::string mFixedBlocksString{""};
    std::vector<std::string> mOutputFieldNames;
    double mIsoValue{-1.0};
    int mNumBufferLayers{-1};
    double mMinEdgeLength{-1.0};
    bool mReadSpreadFile{false};
    bool mRemoveIslands{true};
    bool mConcatenateResults{false};
    bool mAllowNonmanifoldConnections{false};
    bool mIsoOnly{true};
};

} //namespace prune

#endif /* ISO_PRUNE_MAIN_PERCEPTPRUNE_HPP_ */
