#include "plato/optimizer/OptimizerFactory.hpp"

#include <ROL_LineSearchStep.hpp>
#include <ROL_StatusTest.hpp>

namespace plato::optimizer
{
namespace
{
template <typename T>
struct ParameterAndDefault
{
    std::string_view mParameterName;
    T mDefault;
};

constexpr auto kIterationLimit = ParameterAndDefault<int>{/*.mParameterName=*/"Iteration Limit", /*.mDefault=*/10};
constexpr auto kGradientTolerance =
    ParameterAndDefault<double>{/*.mParameterName=*/"Gradient Tolerance", /*.mDefault=*/1.0e-12};
constexpr auto kStepTolerance =
    ParameterAndDefault<double>{/*.mParameterName=*/"Step Tolerance", /*.mDefault=*/1.0e-14};

template <typename ParsedType, typename DefaultType>
void set_status_test_parameter(ROL::ParameterList& aParlist,
                               const boost::optional<ParsedType>& aParameter,
                               const ParameterAndDefault<DefaultType>& aParameterDefault)
{
    aParlist.sublist("Status Test")
        .set<DefaultType>(std::string{aParameterDefault.mParameterName},
                          aParameter.value_or(aParameterDefault.mDefault));
}
}  // anonymous namespace

ROL::ParameterList rol_parameter_list(const ValidOptimizationParameters& aOptimizationParameters)
{
    /// @todo Would like to make this first load from a file if present,
    /// then override or set any parameters specified in the input block,
    /// then fill in any missing entries with defaults.
    if (aOptimizationParameters.rawInput().input_file_name)
    {
        return *ROL::getParametersFromXmlFile(aOptimizationParameters.rawInput().input_file_name.value().mName);
    }
    else
    {
        ROL::ParameterList tParlist;
        tParlist.sublist("Step").sublist("Line Search").sublist("Descent Method").set("Type", "Newton-Krylov");
        set_status_test_parameter(tParlist, aOptimizationParameters.rawInput().max_iterations, kIterationLimit);
        set_status_test_parameter(tParlist, aOptimizationParameters.rawInput().gradient_tolerance, kGradientTolerance);
        set_status_test_parameter(tParlist, aOptimizationParameters.rawInput().step_tolerance, kStepTolerance);
        return tParlist;
    }
}

ROL::Solver<double> make_rol_solver(Teuchos::ParameterList& aROLOptions, ROL::Ptr<ROL::Problem<double>> aROLProblem)
{
    return ROL::Solver<double>{std::move(aROLProblem), aROLOptions};
}

}  // namespace plato::optimizer
