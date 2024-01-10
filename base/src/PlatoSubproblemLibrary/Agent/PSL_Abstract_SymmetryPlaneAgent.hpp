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

// PlatoSubproblemLibraryVersion(7): a stand-alone library for the kernel filter for plato.
#pragma once

/* Abstract helper class for Filter library.
 */

#include "PSL_ParameterDataEnums.hpp"

#include <vector>
#include <cstddef>

namespace PlatoSubproblemLibrary
{
namespace AbstractInterface
{
class PointCloud;
}
class PointCloud;
class ParameterData;
class AbstractAuthority;

class Abstract_SymmetryPlaneAgent
{
public:
    Abstract_SymmetryPlaneAgent(symmetry_plane_agent_t::symmetry_plane_agent_t type,
                                AbstractAuthority* authority);
    virtual ~Abstract_SymmetryPlaneAgent();

    // mirror points by symmetry plane, store mapping internally
    virtual PointCloud* build_kernel_points(double support_distance,
                                            double point_resolution_tolerance,
                                            ParameterData* input_data,
                                            AbstractInterface::PointCloud* all_points,
                                            const std::vector<size_t>& indexes_of_local_points) = 0;
    // expand field data to mirrored points
    virtual std::vector<double> expand_with_symmetry_points(const std::vector<double>& contracted_data_vector) = 0;
    // contract field data by mirrored points
    virtual std::vector<double> contract_by_symmetry_points(const std::vector<double>& expanded_data_vector) = 0;

protected:
    symmetry_plane_agent_t::symmetry_plane_agent_t m_type;
    AbstractAuthority* m_authority;

};

}
