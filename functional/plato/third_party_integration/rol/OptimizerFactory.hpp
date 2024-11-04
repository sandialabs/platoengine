#ifndef PLATO_ROL_INTEGRATION_OPTIMIZERFACTORY
#define PLATO_ROL_INTEGRATION_OPTIMIZERFACTORY

#include <ROL_Algorithm.hpp>
#include <ROL_Solver.hpp>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/third_party_integration/rol/OptimizationParameters.hpp"

namespace plato::third_party_integration::rol
{
using ValidOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::rol_optimization>;

///@brief Create a ROL solver based on the ROL problem and the ROL options specified in the solver parameter list
///
///@param aROLOptions
///@param aProblem
///@return ROL::Solver<double>
[[nodiscard]] ROL::Solver<double> make_rol_solver(Teuchos::ParameterList& aROLOptions,
                                                  const ROL::Ptr<ROL::Problem<double>>& aProblem);

///@brief Generate a Parameters for ROL. Either from input deck or file.
///@param aOptimizationParameters
///@return Parameters
[[nodiscard]] OptimizationParameters make_optimization_parameters(
    const ValidOptimizationParameters& aOptimizationParameters);

}  // namespace plato::third_party_integration::rol

#endif
