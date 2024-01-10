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
 * Plato_Test_AugmentedLagrangian.cpp
 *
 *  Created on: Oct 21, 2017
 */

#include "gtest/gtest.h"

#include "Plato_UnitTestUtils.hpp"

#include "Plato_Radius.hpp"
#include "Plato_Rosenbrock.hpp"
#include "Plato_GoldsteinPrice.hpp"
#include "Plato_Himmelblau.hpp"
#include "Plato_ShiftedEllipse.hpp"

#include "Plato_LinearAlgebra.hpp"
#include "Plato_StandardMultiVector.hpp"

namespace PlatoTest
{

TEST(PlatoTest, RosenbrockCriterion)
{
    double tValue = 2;
    const size_t tNumVectors = 1;
    const size_t tNumControls = 2;
    Plato::StandardMultiVector<double> tControl(tNumVectors, tNumControls, tValue);

    Plato::Rosenbrock<double> tCriterion;
    // TEST OBJECTIVE FUNCTION EVALUATION
    double tObjectiveValue = tCriterion.value(tControl);
    const double tGoldValue = 401;
    const double tTolerance = 1e-6;
    EXPECT_NEAR(tGoldValue, tObjectiveValue, tTolerance);

    // TEST GRADIENT EVALUATION FUNCTION
    Plato::StandardMultiVector<double> tGradient(tNumVectors, tNumControls);
    tCriterion.gradient(tControl, tGradient);
    Plato::StandardMultiVector<double> tGoldVector(tNumVectors, tNumControls);
    const size_t tVectorIndex = 0;
    tGoldVector(tVectorIndex, 0) = 1602;
    tGoldVector(tVectorIndex, 1) = -400;
    PlatoTest::checkMultiVectorData(tGradient, tGoldVector);

    // TEST HESSIAN TIMES VECTOR FUNCTION
    tValue = 1;
    Plato::StandardMultiVector<double> tVector(tNumVectors, tNumControls, tValue);
    Plato::StandardMultiVector<double> tHessianTimesVector(tNumVectors, tNumControls);
    tCriterion.hessian(tControl, tVector, tHessianTimesVector);
    tGoldVector(tVectorIndex, 0) = 3202;
    tGoldVector(tVectorIndex, 1) = -600;
    PlatoTest::checkMultiVectorData(tHessianTimesVector, tGoldVector);
}

TEST(PlatoTest, RadiusCriterion)
{
    const double tValue = 0.5;
    const size_t tNumVectors = 1;
    const size_t tNumControls = 2;
    Plato::StandardMultiVector<double> tControl(tNumVectors, tNumControls, tValue);

    Plato::Radius<double> tCriterion;

    // TEST OBJECTIVE FUNCTION EVALUATION
    double tObjectiveValue = tCriterion.value(tControl);
    const double tGoldValue = -0.5;
    const double tTolerance = 1e-6;
    EXPECT_NEAR(tGoldValue, tObjectiveValue, tTolerance);

    // TEST GRADIENT EVALUATION FUNCTION
    Plato::StandardMultiVector<double> tGradient(tNumVectors, tNumControls);
    tCriterion.gradient(tControl, tGradient);
    Plato::StandardMultiVector<double> tGoldVector(tNumVectors, tNumControls);
    Plato::fill(1., tGoldVector);
    PlatoTest::checkMultiVectorData(tGradient, tGoldVector);

    // TEST HESSIAN TIMES VECTOR FUNCTION
    Plato::StandardMultiVector<double> tVector(tNumVectors, tNumControls, tValue);
    const size_t tVectorIndex = 0;
    tVector(tVectorIndex, 1) = -2.;
    Plato::StandardMultiVector<double> tHessianTimesVector(tNumVectors, tNumControls);
    tCriterion.hessian(tControl, tVector, tHessianTimesVector);
    tGoldVector(tVectorIndex, 0) = 1.0;
    tGoldVector(tVectorIndex, 1) = -4.0;
    PlatoTest::checkMultiVectorData(tHessianTimesVector, tGoldVector);
}

} // namespace PlatoTest
