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

// PlatoSubproblemLibraryVersion(3): a stand-alone library for the kernel filter for plato.
#pragma once

/* Default implementation of mesh scale agent.
 *
 * Determine global mesh scales by considering axis-aligned bounding boxes
 * of all elements in the mesh. Minimum, mean, and maximum scales are computed.
 *
 * Only optimization blocks are included in mesh scale calculations.
 * The filter only affects density values in optimization blocks.
 * Non-optimization blocks are often coarsely meshed.
 */

#include "PSL_Abstract_MeshScaleAgent.hpp"

#include <vector>

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class PointCloud;
}
class AbstractAuthority;

class ByOptimizedElementSide_MeshScaleAgent : public Abstract_MeshScaleAgent
{
public:
    ByOptimizedElementSide_MeshScaleAgent(AbstractAuthority* authority,
                                          AbstractInterface::PointCloud* points);

    double get_mesh_minimum_scale() override;
    double get_mesh_average_scale() override;
    double get_mesh_maximum_scale() override;

protected:
    void calculate_mesh_scales();
    std::vector<bool> get_is_optimizable();

    bool m_calculated_mesh_scales;
    double m_mesh_minimum_scale;
    double m_mesh_average_scale;
    double m_mesh_maximum_scale;

};

}
