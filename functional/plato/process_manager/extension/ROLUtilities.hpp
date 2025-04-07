#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLUTILITIES
#define PLATO_PROCESSMANAGER_EXTENSION_ROLUTILITIES

#include <ROL_Solver.hpp>
#include <ROL_StdVector.hpp>
#include <memory>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/third_party_integration/rol/OptimizationParameters.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::geometry::library
{
class OutputManager;
}

namespace plato::process_manager::extension
{
///@brief Create a ROL objective object from a ProcessManagerData by composing the mGeometryFunction with the mObjective
/// @a aOutputManager is used to manage output of design variables through the output function of mGeometry
[[nodiscard]] auto make_rol_objective(
    const library::ProcessManagerData& aProblem,
    geometry::library::OutputManager aOutputManager = geometry::library::OutputManager{})
    -> std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction>;

///@brief Create a ROL constraint from a ProcessManagerData by looping through the constraints
[[nodiscard]] auto make_rol_constraints(const library::ProcessManagerData& aProblem)
    -> std::vector<third_party_integration::rol::ROLConstraint>;

/// @brief Create the fully posed ROL problem from the ProcessManagerData struct. Apply bound constraints, constraints,
/// and finalize the ROL problem.
/// @return The returned `ROL::StdVector` will hold the solution to the optimization problem after `solve` is called on
/// the returned `ROL::Problem`.
/// @note The solution vector is returned to avoid issues with ROL changing derived types. If an inequality constraint
/// is used, the type returned from `getPrimalOptimizationVector` will be `ROL::PartitionedVector`, which is not easy to
/// copy to a `std::vector`.
[[nodiscard]] auto make_rol_problem(
    const library::ProcessManagerData& aProblem,
    geometry::library::OutputManager aOutputManager = geometry::library::OutputManager{})
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>;

[[nodiscard]] auto make_rol_problem(const library::ProcessManagerData& aProblem,
                                    const ROL::Ptr<ROL::StdObjective<double>>& aROLObjective)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>;

using ValidOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::rol_optimization>;

///@brief Create a ROL solver based on the ROL problem and the ROL options specified in the solver parameter list
[[nodiscard]] auto make_rol_solver(Teuchos::ParameterList& aROLOptions, const ROL::Ptr<ROL::Problem<double>>& aProblem)
    -> ROL::Solver<double>;

///@brief Generate a Parameters for ROL. Either from input deck or file.
[[nodiscard]] auto make_optimization_parameters(const ValidOptimizationParameters& aOptimizationParameters)
    -> third_party_integration::rol::OptimizationParameters;

///@brief Generate a Parameters for ROL. Either from input deck or file.
[[nodiscard]] auto make_optimization_parameters(const library::NewValidatedProcessManagerInput& aOptimizationParameters)
    -> third_party_integration::rol::OptimizationParameters;

}  // namespace plato::process_manager::extension

#endif
