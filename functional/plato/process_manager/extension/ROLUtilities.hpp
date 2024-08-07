#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLUTILITIES
#define PLATO_PROCESSMANAGER_EXTENSION_ROLUTILITIES

#include <ROL_StdVector.hpp>
#include <memory>

#include "plato/third_party_integration/rol/OptimizerFactory.hpp"
#include "plato/third_party_integration/rol/ROLConstraint.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{
///@brief Create a ROL objective object from a ProcessManagerData by composing the mGeometryFunction with the mObjective
[[nodiscard]] auto make_rol_objective(const library::ProcessManagerData& aProblem)
    -> std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction>;

///@brief Create a ROL constraint from a ProcessManagerData by looping through the constraints
[[nodiscard]] auto make_rol_constraints(const library::ProcessManagerData& aProblem)
    -> std::vector<third_party_integration::rol::ROLConstraint>;

///@brief Create the fully posed ROL problem from the ProcessManagerData struct
/// Apply bound constraints, constraints, and finalize the ROL problem
[[nodiscard]] auto make_rol_problem(const library::ProcessManagerData& aProblem)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>;

///@brief Create a ROL problem from the ProcessManagerData struct
/// Apply bound constraints, but ignore equality/inequality constraints, and then finalize the ROL problem
[[nodiscard]] auto make_rol_problem_without_constraints_for_gradient_check(const library::ProcessManagerData& aProblem)
    -> std::pair<ROL::Ptr<ROL::Problem<double>>, ROL::Ptr<ROL::StdVector<double>>>;

}  // namespace plato::process_manager::extension

#endif
