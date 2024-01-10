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
 * Plato_Test_XTK_Interface.cpp
 *
 *  Created on: Jan 22, 2019
 *      Author: ktdoble
 */
#include <gtest/gtest.h>
#include <typedefs.hpp>
#include "Plato_XTK_Application.hpp"
#include "Plato_XTK_SharedData.hpp"

// MORIS XTK INCLUDES
#include <cl_Param_List.hpp>
#include <fn_PRM_XTK_Parameters.hpp>
#include <fn_PRM_GEN_Parameters.hpp>
#include <fn_PRM_HMR_Parameters.hpp>

#include <cl_HMR.hpp>

#include <cl_Cell.hpp>
#include <typedefs.hpp>
#include <memory>

#include "fn_stringify_matrix.hpp"


namespace PlatoTestXTK
{
  TEST(PlatoTestXTK, XTK)
  {

        std::string tXTKInput = "./xtkInput.xml";
        std::string tAppFile = "./xtkApp.xml";

        MPI_Comm tComm = MPI_COMM_WORLD;

        Plato::XTKApplication tXTKApplication(tXTKInput, tAppFile, tComm);

        tXTKApplication.initialize();

        tXTKApplication.compute("Update Problem");

        // test some information in the background mesh
        moris::uint tNumBGNodes = tXTKApplication.mCurrentBGMTK->get_interpolation_mesh(0)->get_num_nodes();
        moris::uint tNumBGElems = tXTKApplication.mCurrentBGMTK->get_interpolation_mesh(0)->get_num_elems();

        EXPECT_TRUE(tNumBGNodes == 4125);
        EXPECT_TRUE(tNumBGElems == 17090);

        moris::uint tNumOutputNodes = tXTKApplication.mCurrentOutputMTK->get_integration_mesh(0)->get_num_nodes();
        moris::uint tNumOutputElems = tXTKApplication.mCurrentOutputMTK->get_integration_mesh(0)->get_num_elems();


        EXPECT_TRUE(tNumBGNodes == 4125);
        EXPECT_TRUE(tNumBGElems == 17090);

        moris::Cell<std::string> tBlockSetNames = tXTKApplication.mCurrentOutputMTK->get_integration_mesh(0)->get_block_set_names();


        EXPECT_TRUE(tBlockSetNames(0) == "block_1");
        EXPECT_TRUE(tBlockSetNames(1) == "block_2");

        uint tNumSideSets = tXTKApplication.mCurrentOutputMTK->get_integration_mesh(0)->get_num_side_sets();

        EXPECT_TRUE(tNumSideSets == 2);

  }
}
