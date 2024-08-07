#ifndef PLATO_ROL_INTEGRATION_ROLCONSTRAINT
#define PLATO_ROL_INTEGRATION_ROLCONSTRAINT

#include <ROL_Bounds.hpp>
#include <ROL_Problem.hpp>
#include <ROL_StdVector.hpp>
#include <string>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/third_party_integration/rol/ROLVectorConstraintFunction.hpp"

namespace plato::third_party_integration::rol
{

struct ROLConstraint
{
    std::string mName;
    unsigned int mNumberOfConstraints = 1;
    bool mLinear = false;
    criteria::library::ConstraintType mType;
    std::unique_ptr<ROLVectorConstraintFunction> mConstraintFunction;
};

void add_constraint_to_problem(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint);

/// @brief Helper function that creates a bounds object that given a pair @a aBounds which has the lower and upper
/// bounds for all design variables.
[[nodiscard]] auto create_rol_bound_constraint(const std::pair<std::vector<double>, std::vector<double>>& aBounds)
    -> ROL::Ptr<ROL::Bounds<double>>;

namespace detail
{
void add_linear_equality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint);

void add_equality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint);

void add_linear_inequality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint);

void add_inequality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint);

/// @brief Helper function that creates a bounds object sized to @a aNumberofConstraints that will work for vector
/// valued constraints all less/greater than or equal to 0
[[nodiscard]] auto create_inequality_bounds(const criteria::library::ConstraintType& aType,
                                            const unsigned int aNumberOfConstraints) -> ROL::Ptr<ROL::Bounds<double>>;

[[nodiscard]] auto create_greater_than_inequality_bounds(const unsigned int aNumberOfConstraints)
    -> ROL::Ptr<ROL::Bounds<double>>;

[[nodiscard]] auto create_less_than_inequality_bounds(const unsigned int aNumberOfConstraints)
    -> ROL::Ptr<ROL::Bounds<double>>;

enum class ConstraintCombination
{
    kLinearEquality,
    kLinearInequality,
    kNonlinearEquality,
    kNonlinearInequality
};

[[nodiscard]] auto determine_constraint_combination(const ROLConstraint& aConstraint) -> ConstraintCombination;

}  // namespace detail
}  // namespace plato::third_party_integration::rol
#endif
