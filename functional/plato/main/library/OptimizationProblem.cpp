#include "plato/main/library/OptimizationProblem.hpp"

#include <ROL_Algorithm.hpp>
#include <fstream>
#include <string_view>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/main/library/PlatoProblem.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/rol_integration/ROLHelpers.hpp"

namespace plato::main::library
{
namespace
{
constexpr std::string_view kROLGradientCheckFileName = "ROL_Gradient_Check.txt";
constexpr std::string_view kROLSensitivityCheckFileName = "ROL_Sensitivity_Check.txt";
constexpr std::string_view kROLOptimizerFileName = "ROL_Optimizer.txt";
constexpr std::string_view kROLConstraintCheckFileName = "ROL_Constraint_Check.txt";

///@brief Helper function that creates a perturbation of the initial value for use in diagnostic checks
ROL::StdVector<double> generatePerturbation(const int aDimension)
{
    ROL::StdVector<double> tPerturbation(aDimension);
    tPerturbation.randomize();
    return tPerturbation;
}

}  // namespace

OptimizationProblem::OptimizationProblem(const std::string_view aInputFile)
    : mProblem(make_plato_problem(parse_and_validate_from_file(aInputFile))),
      mROLProblem(make_rol_problem(mProblem).release()),
      mROLSolver(plato::optimizer::make_rol_solver(mProblem.mROLOptions, mROLProblem))
{
}

void OptimizationProblem::gradientCheck() const
{
    std::ofstream tOutFile(std::string{kROLGradientCheckFileName});
    constexpr bool tPrintOutput = true;

    mROLProblem->getObjective()->checkGradient(rol_integration::to_rol_vector(mProblem.mGeometry.mInitialGuess),
                                               generatePerturbation(dimension()), tPrintOutput, tOutFile);
}

void OptimizationProblem::constraintCheck() const
{
    // This mutates the initial guess, so it shouldn't be used as is before
    // an optimization run. Calling clone on each vector would be a solution, but doing so
    // on the result of getResidualVector seg faults
    constexpr double tPerturbationScale = 0.1;
    auto tPrimal = mROLProblem->getPrimalOptimizationVector();
    tPrimal->randomize(0, tPerturbationScale);
    auto tv = mROLProblem->getPrimalOptimizationVector();
    tv->randomize(0, tPerturbationScale);
    auto tResidual = mROLProblem->getResidualVector();
    tResidual->randomize(-tPerturbationScale, tPerturbationScale);
    auto tMultiplier = mROLProblem->getMultiplierVector();
    tMultiplier->randomize(-tPerturbationScale, tPerturbationScale);

    auto tConstraint = mROLProblem->getConstraint();
    if (tConstraint)
    {
        constexpr bool tPrintOutput = true;
        std::ofstream tOutput(std::string{kROLConstraintCheckFileName});
        tConstraint->checkApplyJacobian(*tPrimal, *tv, *tResidual, tPrintOutput, tOutput);
        tConstraint->checkAdjointConsistencyJacobian(*tMultiplier, *tv, *tPrimal, tPrintOutput, tOutput);
        mROLProblem->checkLinearity(tPrintOutput, tOutput);
    }
    else
    {
        std::cout << "No constraints, skipping constraint check.\n";
    }
}

void OptimizationProblem::sensitivityCheck() const
{
    std::ofstream tOutFile(std::string{kROLSensitivityCheckFileName});
    constexpr bool tPrintOutput = true;

    auto tSensitivityObjective = make_rol_sensitivity_objective(mProblem);
    tSensitivityObjective->checkGradient(rol_integration::to_rol_vector(mProblem.mGeometry.mInitialGuess),
                                         generatePerturbation(dimension()), tPrintOutput, tOutFile);
}

void OptimizationProblem::optimize()
{
    std::ofstream tOutFile(std::string{kROLOptimizerFileName});
    mROLSolver.solve(tOutFile);
    outputResult();
}

void OptimizationProblem::outputResult() const
{
    if (mCommunicator.rank() == 0)
    {
        mProblem.mGeometry.mOutput(rol_integration::to_dynamic_vector(*mROLProblem->getPrimalOptimizationVector()));
    }
}

int OptimizationProblem::dimension() const { return mProblem.mGeometry.mInitialGuess.size(); }

}  // namespace plato::main::library
