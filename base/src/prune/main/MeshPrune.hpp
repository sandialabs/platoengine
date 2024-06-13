/*
 * MeshPrune.hpp
 *
 *  Created on: Jul 28, 2017
 *      Author: tzirkle
 */

#ifndef ISO_PRUNE_MAIN_MESHPRUNE_HPP_
#define ISO_PRUNE_MAIN_MESHPRUNE_HPP_

#include "MeshManager.hpp"
#include <percept/PerceptMesh.hpp>
#include "PerceptPrune.hpp"

class MeshPrune
{
public:
    void prune_mesh(MeshManager &tMeshManager);
};



#endif /* ISO_PRUNE_MAIN_MESHPRUNE_HPP_ */
