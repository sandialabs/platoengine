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

#pragma once

#include <mpi.h>
#include <cstddef>
#include "PlatoEngine_AbstractFilter.hpp"

namespace PlatoSubproblemLibrary
{
class AbstractProjectionFilter;
class ParameterData;
class AbstractAuthority;
namespace AbstractInterface
{
class PointCloud;
class ParallelExchanger;
}
}
class DataMesh;

namespace Plato
{
class InputData;

class AbstractProjectionFilter : public AbstractFilter
{
public:
    AbstractProjectionFilter();
    ~AbstractProjectionFilter();

    //Filter functions
    void build(InputData aInputData, MPI_Comm& aLocalComm, DataMesh* aMesh) override;
    void apply_on_field(size_t length, double* field_data) override;
    void apply_on_gradient(size_t length, double* base_field_data, double* gradient_data) override;
    void advance_continuation() override;

private:

    virtual void allocateFilter() = 0;

    void build_input_data(InputData interface);
    void build_points(DataMesh* mesh);
    void build_parallel_exchanger(DataMesh* mesh);

    MPI_Comm m_comm;

    int  mAdvanceContinuationIteration;
    int  mStartIteration;
    int  mUpdateInterval;
    bool mAdditiveContinuation;

protected:

    PlatoSubproblemLibrary::AbstractProjectionFilter* m_filter;
    PlatoSubproblemLibrary::AbstractAuthority* m_authority;
    PlatoSubproblemLibrary::ParameterData* m_input_data;
    PlatoSubproblemLibrary::AbstractInterface::PointCloud* m_points;
    PlatoSubproblemLibrary::AbstractInterface::ParallelExchanger* m_parallel_exchanger;

};

}
