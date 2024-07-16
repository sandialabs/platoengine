#ifndef PLATO_ROL_INTEGRATION_OPTIMIZERFACTORY
#define PLATO_ROL_INTEGRATION_OPTIMIZERFACTORY

#include <ROL_Algorithm.hpp>
#include <ROL_Solver.hpp>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"

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

///@brief Generate a ROL::ParameterList. Either from input deck or file.
/// First attempt is to load from a file, but if the file input is not specified the defaults
/// or values specified in the input block will be used
///@param aOptimizationParameters
///@return ROL::ParameterList
[[nodiscard]] ROL::ParameterList rol_parameter_list(const ValidOptimizationParameters& aOptimizationParameters);

}  // namespace plato::third_party_integration::rol

#endif
