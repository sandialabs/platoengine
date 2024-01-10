#pragma once

#include "ROL_Ptr.hpp"
#include "ROL_Vector.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace Plato
{

namespace GradientCheck
{

template<typename ScalarType>
const ROL::Ptr<ROL::Vector<ScalarType>>
perturb_control
(const ROL::Ptr<ROL::Vector<ScalarType>> & aControl,
 double aScale,
 unsigned int aSeed = 0)
{
    auto tPerturbation = aControl->clone();

    if(aSeed != 0)
    {
        std::srand(aSeed);
        std::cout << "Setting seed to: " << aSeed <<std::endl;
    }
    tPerturbation->randomize(-aScale, aScale);

    return tPerturbation;
}

template<typename ScalarType>
const std::vector<ScalarType>
generate_steps
(unsigned int aNumSteps,
 unsigned int aStepSize = 10)
{
    std::vector<ScalarType> tSteps(aNumSteps, 1.0);
    for(unsigned int iStep = 1; iStep<aNumSteps; iStep++)
    {
        tSteps[iStep] = tSteps[iStep-1] / aStepSize;
    }

    return tSteps;
}

}

}
