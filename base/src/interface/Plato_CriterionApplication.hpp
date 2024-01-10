#ifndef SRC_OBJECTIVEAPPLICATION_HPP_
#define SRC_OBJECTIVEAPPLICATION_HPP_

#include "Plato_Application.hpp"

namespace Plato{
/// An Application that implements the criterion constrained interface.
///
/// The purpose of this class is to be a base class for objective-based applications.
/// I.e. applications that only represent objective functions and not constraints or
/// other operations.
class CriterionApplication : public Application
{
public:
    void computeCriterionValue() override = 0; 
    void computeCriterionGradient() override = 0;
    void computeCriterionHessianTimesVector() override = 0;
    std::vector<OperationType> supportedOperationTypes() const override
    { 
        return {OperationType::kCriterionValue, OperationType::kCriterionGradient, OperationType::kCriterionHessian};
    }
};
}
#endif