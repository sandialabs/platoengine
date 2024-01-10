/*
 OptimizationAlgorithmFactory.hpp
 *
 *  Created on: May 9, 2022
 */
#pragma once

#include <string>
#include <memory>
#include "XMLGeneratorUtilities.hpp"
#include "OptimizationAlgorithm.hpp"

namespace director
{

class OptimizationAlgorithmFactory
{
public:
    std::shared_ptr<OptimizationAlgorithm> create(const XMLGen::InputData& aMetaData);
};


}