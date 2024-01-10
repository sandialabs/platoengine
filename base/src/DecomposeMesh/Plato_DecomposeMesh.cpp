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

/*
 * Plato_DecomposeMesh.cpp
 *
 */

#ifdef STK_ENABLED
#include "Plato_DecomposeMesh.hpp"
#include <stk_io/StkMeshIoBroker.hpp>
#endif

#include <string>

namespace Plato
{

void decomposeMesh(MPI_Comm aComm, const std::string& aMeshFile)
{
#ifdef STK_ENABLED
    int tMyRank;
    int tSize;
    MPI_Comm_rank(aComm, &tMyRank);
    MPI_Comm_size(aComm, &tSize);

    if (tSize <= 1) return;

    // Read the serial mesh
    stk::io::StkMeshIoBroker ioBroker(aComm);
    ioBroker.use_simple_fields();
    ioBroker.property_add(Ioss::Property("DECOMPOSITION_METHOD", "RCB"));
    ioBroker.add_mesh_database(aMeshFile, stk::io::READ_MESH);
    ioBroker.create_input_mesh();
    ioBroker.add_all_mesh_fields_as_input_fields();
    ioBroker.populate_bulk_data();
    std::vector<double> tTimeSteps = ioBroker.get_time_steps();
    size_t index = ioBroker.create_output_mesh(aMeshFile, stk::io::WRITE_RESULTS);
    ioBroker.set_active_mesh(index);
    stk::mesh::FieldVector all_fields = ioBroker.meta_data().get_fields();
    for(size_t j=0; j<all_fields.size(); ++j)
    {
        stk::mesh::FieldBase* cur_field = all_fields[j];
        const Ioss::Field::RoleType* tRoleType = stk::io::get_field_role(*cur_field);
        if(tRoleType && *tRoleType == Ioss::Field::TRANSIENT)
            ioBroker.add_field(index, *cur_field);
    }
    for(size_t i=0; i<tTimeSteps.size(); ++i)
    {
        ioBroker.read_defined_input_fields((int)(i+1));
        ioBroker.begin_output_step(index, tTimeSteps[i]);
        ioBroker.write_defined_output_fields(index);
        ioBroker.end_output_step(index);
    }

    ioBroker.write_output_mesh(index);
    MPI_Barrier(aComm);
#endif
}


} // end namespace Plato
