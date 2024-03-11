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
#include "Plato_Utils.hpp"
#include "Plato_InputData.hpp"
#include "Plato_EnforceBounds.hpp"
#include "Plato_SystemCallOperation.hpp"
#include "Plato_OperationsUtilities.hpp"

#include <fstream>
#include <gtest/gtest.h>

namespace PlatoTestOperations
{

TEST(EnforceBounds, applyBounds)
{
    Plato::InputData tNode;
    Plato::EnforceBounds tEnforceBounds(NULL, tNode);

    int tLength = 10;
    double tDataToBound[10] = {1, -3, 5, 10, 2, 7, 22, 33, 0, 0};
    double tLowerBound[10] =  {1.5, 0, 5, -10, 3, 7, -1, 14, -1, -1};
    double tUpperBound[10] =  {2, 1, 5, 1, 3.3, 7.5, 0, 15, 0, 1};

    tEnforceBounds.applyBounds(tLength, tLowerBound, tUpperBound, tDataToBound); 

    EXPECT_EQ(tDataToBound[0], 1.5);
    EXPECT_EQ(tDataToBound[1], 0);
    EXPECT_EQ(tDataToBound[2], 5);
    EXPECT_EQ(tDataToBound[3], 1);
    EXPECT_EQ(tDataToBound[4], 3);
    EXPECT_EQ(tDataToBound[5], 7);
    EXPECT_EQ(tDataToBound[6], 0);
    EXPECT_EQ(tDataToBound[7], 15);
    EXPECT_EQ(tDataToBound[8], 0);
    EXPECT_EQ(tDataToBound[9], 0);
}

} // end PlatoTestOperations namespace
