#include "plato/third_party_integration/rol/OptimizerFactory.hpp"

#include <ROL_LineSearchStep.hpp>
#include <ROL_StatusTest.hpp>

#include "plato/third_party_integration/rol/OptimizationParameters.hpp"
#include "plato/utilities/BoostOptionalToStdOptional.hpp"

namespace plato::third_party_integration::rol
{
namespace
{

[[nodiscard]] auto load_file_or_use_default_parameters(const ValidOptimizationParameters& aOptimizationParameters)
    -> third_party_integration::rol::OptimizationParameters
{
    if (aOptimizationParameters.rawInput().input_file_name)
    {
        return third_party_integration::rol::OptimizationParameters(
            aOptimizationParameters.rawInput().input_file_name.value().mToken);
    }
    return third_party_integration::rol::OptimizationParameters();
}

void apply_verbose_output(const ValidOptimizationParameters& aOptimizationParameters,
                          OptimizationParameters& aParameters)
{
    if (aOptimizationParameters.rawInput().verbose_output && aOptimizationParameters.rawInput().verbose_output.value())
    {
        aParameters.verbose();
    }
}

void apply_approximate_hessian(const ValidOptimizationParameters& aOptimizationParameters,
                               OptimizationParameters& aParameters)
{
    if (aOptimizationParameters.rawInput().approximate_hessian &&
        aOptimizationParameters.rawInput().approximate_hessian.value())
    {
        aParameters.approximateHessian();
    }
}

void write_parameters(const ValidOptimizationParameters& aOptimizationParameters, OptimizationParameters& aParameters)
{
    if (aOptimizationParameters.rawInput().export_settings_file_name)
    {
        aParameters.writeParameters(aOptimizationParameters.rawInput().export_settings_file_name.value().mToken);
    }
}

}  // anonymous namespace

OptimizationParameters make_optimization_parameters(const ValidOptimizationParameters& aOptimizationParameters)
{
    third_party_integration::rol::OptimizationParameters tParameters =
        load_file_or_use_default_parameters(aOptimizationParameters);

    tParameters.maximumIterations(utilities::to_std_optional(aOptimizationParameters.rawInput().max_iterations));
    tParameters.gradientTolerance(utilities::to_std_optional(aOptimizationParameters.rawInput().gradient_tolerance));
    tParameters.stepTolerance(utilities::to_std_optional(aOptimizationParameters.rawInput().step_tolerance));
    tParameters.initialSearchRadius(
        utilities::to_std_optional(aOptimizationParameters.rawInput().initial_search_radius));

    apply_verbose_output(aOptimizationParameters, tParameters);
    apply_approximate_hessian(aOptimizationParameters, tParameters);
    write_parameters(aOptimizationParameters, tParameters);

    return tParameters;
}

ROL::Solver<double> make_rol_solver(Teuchos::ParameterList& aROLOptions,
                                    const ROL::Ptr<ROL::Problem<double>>& aROLProblem)
{
    return ROL::Solver<double>{aROLProblem, aROLOptions};
}

}  // namespace plato::third_party_integration::rol
