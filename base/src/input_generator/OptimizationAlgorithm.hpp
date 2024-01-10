/*
 *  OptimizationAlgorithm.hpp
 *
 *  Created on: May 9, 2022
 */
#pragma once

#include <string>
#include "pugixml.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "Stage.hpp"

namespace director
{

class OptimizationAlgorithm
{
protected:
   using StagePtr = std::shared_ptr<Stage>;
   std::string mPackage;
   std::string mMaxIterations;

   void appendOutputStage(pugi::xml_node& aNode);
   void appendOptimizationVariables(pugi::xml_node& aNode,
                                    StagePtr aInitialization,
                                    StagePtr aUpperBound,
                                    StagePtr aLowerBound);
   void appendObjectiveData(pugi::xml_node& aNode,
                            StagePtr aObjectiveValue,
                            StagePtr aObjectiveGradient);
   void appendConstraintData(pugi::xml_node& aNode,
                             StagePtr aConstraintValue,
                             StagePtr aConstraintGradient);
   void appendBoundsData(pugi::xml_node& aNode);

public:
    OptimizationAlgorithm(const XMLGen::OptimizationParameters& aParameters);
    virtual ~OptimizationAlgorithm() = default;

    virtual void writeInterface(pugi::xml_node& aNode,
                                StagePtr aInitialization = nullptr,
                                StagePtr aUpperBound = nullptr,
                                StagePtr aLowerBound = nullptr,
                                StagePtr aObjectiveValue = nullptr,
                                StagePtr aObjectiveGradient = nullptr,
                                StagePtr aConstraintValue = nullptr,
                                StagePtr aConstraintdient = nullptr) = 0;
    virtual void writeAuxiliaryFiles(pugi::xml_node& aNode) = 0;
};

class OptimizationAlgorithmPlatoOC : public OptimizationAlgorithm
{
private:
    std::string mProblemUpdateFrequency;

public:
    OptimizationAlgorithmPlatoOC(const XMLGen::OptimizationParameters& aParameters);
    void writeInterface(pugi::xml_node& aNode,
                        StagePtr aInitialization = nullptr,
                        StagePtr aUpperBound = nullptr,
                        StagePtr aLowerBound = nullptr,
                        StagePtr aObjectiveValue = nullptr,
                        StagePtr aObjectiveGradient = nullptr,
                        StagePtr aConstraintValue = nullptr,
                        StagePtr aConstraintGradient = nullptr) override;
    void writeAuxiliaryFiles(pugi::xml_node& aNode) override;
};

} //namespace