#include "OptimizationAlgorithm.hpp"
#include "pugixml.hpp"
#include "XMLGeneratorUtilities.hpp"
#include <regex>
using namespace XMLGen;
namespace director
{

OptimizationAlgorithm::OptimizationAlgorithm(const XMLGen::OptimizationParameters& aParameters)
{
    mMaxIterations = aParameters.max_iterations();
}

void OptimizationAlgorithm::appendOutputStage(pugi::xml_node& aNode)
{
    auto tOutput = aNode.append_child("Output");
    addChild(tOutput,"OutputStage","Output To File");
}

void OptimizationAlgorithm::appendOptimizationVariables
(pugi::xml_node& aNode,
 StagePtr aInitialization,
 StagePtr aUpperBound,
 StagePtr aLowerBound)
{ 
    auto tOptimizationVar = aNode.append_child("OptimizationVariables");
    if (aInitialization)
    {
        addChild(tOptimizationVar, "ValueName", aInitialization->outputSharedDataName());
        addChild(tOptimizationVar, "InitializationStage", aInitialization->name());
    }
    if (aLowerBound)
    {
        addChild(tOptimizationVar, "LowerBoundValueName", aLowerBound->inputSharedDataName());
        addChild(tOptimizationVar, "LowerBoundVectorName", aLowerBound->outputSharedDataName());
        addChild(tOptimizationVar, "SetLowerBoundsStage", aLowerBound->name());
    }
    if (aUpperBound)
    {
        addChild(tOptimizationVar, "UpperBoundValueName", aUpperBound->inputSharedDataName());
        addChild(tOptimizationVar, "UpperBoundVectorName", aUpperBound->outputSharedDataName());
        addChild(tOptimizationVar, "SetUpperBoundsStage", aUpperBound->name());
    }
}

void OptimizationAlgorithm::appendObjectiveData
(pugi::xml_node& aNode,
 StagePtr aObjectiveValue,
 StagePtr aObjectiveGradient)
{ 
    auto tObjective = aNode.append_child("Objective");
    if (aObjectiveValue)
    {
        addChild(tObjective, "ValueStageName", aObjectiveValue->name());
        addChild(tObjective, "ValueName", aObjectiveValue->outputSharedDataName());
    }
    if (aObjectiveGradient)
    {
        addChild(tObjective, "GradientStageName", aObjectiveGradient->name());
        addChild(tObjective, "GradientName", aObjectiveGradient->outputSharedDataName());
    }
}

void OptimizationAlgorithm::appendConstraintData
(pugi::xml_node& aNode,
 StagePtr aConstraintValue,
 StagePtr aConstraintGradient)
{ 
    auto tConstraint = aNode.append_child("Constraint");
    if (aConstraintValue)
    {
        addChild(tConstraint, "ValueStageName", aConstraintValue->name());
        addChild(tConstraint, "ValueName", aConstraintValue->outputSharedDataName());
    }
    if (aConstraintGradient)
    {
        addChild(tConstraint, "GradientStageName", aConstraintGradient->name());
        addChild(tConstraint, "GradientName", aConstraintGradient->outputSharedDataName());
    }
    addChild(tConstraint, "NormalizedTargetValue", "");
    addChild(tConstraint, "AbsoluteTargetValue", "");
    addChild(tConstraint, "ReferenceValueName", "");
    addChild(tConstraint, "ReferenceValue", "");
}

void OptimizationAlgorithm::appendBoundsData
(pugi::xml_node& aNode)
{ 
    auto tBounds = aNode.append_child("BoundConstraint");
    addChild(tBounds, "Upper", "1.0");
    addChild(tBounds, "Lower", "0.0");
}

//****************************PLATO **********************************************//
OptimizationAlgorithmPlatoOC::OptimizationAlgorithmPlatoOC(const XMLGen::OptimizationParameters& aParameters)
: OptimizationAlgorithm(aParameters)
{
    mProblemUpdateFrequency = aParameters.problem_update_frequency();
}
void OptimizationAlgorithmPlatoOC::writeInterface
(pugi::xml_node& aNode,
 StagePtr aInitialization,
 StagePtr aUpperBound,
 StagePtr aLowerBound,
 StagePtr aObjectiveValue,
 StagePtr aObjectiveGradient,
 StagePtr aConstraintValue,
 StagePtr aConstraintGradient)
{
    auto tOptimizer = aNode.append_child("Optimizer");
    addChild(tOptimizer, "Package","OC");
    
    auto tOptions = tOptimizer.append_child("Options");    
    addChildCheckEmpty(tOptions, "ProblemUpdateFrequency",mProblemUpdateFrequency);

    auto tConvergence = tOptimizer.append_child("Convergence");
    addChildCheckEmpty(tConvergence, "MaxIterations",mMaxIterations);

    appendOutputStage(tOptimizer);
    appendOptimizationVariables(tOptimizer, aInitialization, aUpperBound, aLowerBound);
    appendObjectiveData(tOptimizer, aObjectiveValue, aObjectiveGradient);
    appendConstraintData(tOptimizer, aConstraintValue, aConstraintGradient);
    appendBoundsData(tOptimizer);
}

void OptimizationAlgorithmPlatoOC::writeAuxiliaryFiles(pugi::xml_node& /*aNode*/)
{

}

}//namespace
