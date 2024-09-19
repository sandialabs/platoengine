#include "plato/third_party_integration/rol/ROLConstraint.hpp"

#include <functional>
#include <map>

#include "plato/third_party_integration/rol/Utilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::third_party_integration::rol
{

using CreateInequalityBoundsFunction = std::function<ROL::Ptr<ROL::Bounds<double>>(const unsigned int)>;

const std::map<criteria::library::ConstraintType, CreateInequalityBoundsFunction> kCreateInequalityBoundsMap = {
    {criteria::library::ConstraintType::kGreaterThan,
     CreateInequalityBoundsFunction{[](const unsigned int aNumberOfConstraints)
                                    { return detail::create_greater_than_inequality_bounds(aNumberOfConstraints); }}},
    {criteria::library::ConstraintType::kLessThan,
     CreateInequalityBoundsFunction{[](const unsigned int aNumberOfConstraints)
                                    { return detail::create_less_than_inequality_bounds(aNumberOfConstraints); }}}};

using AddConstraintFunction = std::function<void(ROL::Problem<double>&, ROLConstraint&)>;
const std::map<detail::ConstraintCombination, AddConstraintFunction> kAddConstraintMap{
    {detail::ConstraintCombination::kLinearEquality,
     AddConstraintFunction{[](ROL::Problem<double>& aProblem, ROLConstraint& aConstraint)
                           { detail::add_linear_equality_constraint(aProblem, aConstraint); }}},
    {detail::ConstraintCombination::kLinearInequality,
     AddConstraintFunction{[](ROL::Problem<double>& aProblem, ROLConstraint& aConstraint)
                           { detail::add_linear_inequality_constraint(aProblem, aConstraint); }}},
    {detail::ConstraintCombination::kNonlinearEquality,
     AddConstraintFunction{[](ROL::Problem<double>& aProblem, ROLConstraint& aConstraint)
                           { detail::add_equality_constraint(aProblem, aConstraint); }}},
    {detail::ConstraintCombination::kNonlinearInequality,
     AddConstraintFunction{[](ROL::Problem<double>& aProblem, ROLConstraint& aConstraint)
                           { detail::add_inequality_constraint(aProblem, aConstraint); }}}};

void add_constraint_to_problem(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint)
{
    if (kAddConstraintMap.find(detail::determine_constraint_combination(aROLConstraint)) != kAddConstraintMap.end())
    {
        kAddConstraintMap.at(detail::determine_constraint_combination(aROLConstraint))(aProblem, aROLConstraint);
    }
    else
    {
        throw utilities::Exception("Request for unknown combination of constraint types.");
    }
}

auto create_rol_bound_constraint(const std::pair<std::vector<double>, std::vector<double>>& aBounds)
    -> ROL::Ptr<ROL::Bounds<double>>
{
    auto tLowerBounds = ROL::makePtr<std::vector<double>>(aBounds.first);
    auto tUpperBounds = ROL::makePtr<std::vector<double>>(aBounds.second);
    auto tLowerBoundsVector = ROL::makePtr<ROL::StdVector<double>>(std::move(tLowerBounds));
    auto tUpperBoundsVector = ROL::makePtr<ROL::StdVector<double>>(std::move(tUpperBounds));
    return ROL::makePtr<ROL::Bounds<double>>(std::move(tLowerBoundsVector), std::move(tUpperBoundsVector));
}

namespace detail
{

namespace
{
void throw_if_number_of_constraints_zero(const unsigned int aNumberOfConstraints)
{
    if (aNumberOfConstraints == 0)
    {
        throw utilities::Exception("Cannot create a 0 length vector for the bounds of constraints.");
    }
}

}  // namespace

auto create_greater_than_inequality_bounds(const unsigned int aNumberOfConstraints) -> ROL::Ptr<ROL::Bounds<double>>
{
    throw_if_number_of_constraints_zero(aNumberOfConstraints);
    return create_rol_bound_constraint({std::vector<double>(aNumberOfConstraints, 0),
                                        std::vector<double>(aNumberOfConstraints, ROL::ROL_INF<double>())});
}

auto create_less_than_inequality_bounds(const unsigned int aNumberOfConstraints) -> ROL::Ptr<ROL::Bounds<double>>
{
    throw_if_number_of_constraints_zero(aNumberOfConstraints);
    return create_rol_bound_constraint({std::vector<double>(aNumberOfConstraints, ROL::ROL_NINF<double>()),
                                        std::vector<double>(aNumberOfConstraints, 0)});
}

auto create_inequality_bounds(const criteria::library::ConstraintType& aType, const unsigned int aNumberOfConstraints)
    -> ROL::Ptr<ROL::Bounds<double>>
{
    if (kCreateInequalityBoundsMap.find(aType) != kCreateInequalityBoundsMap.end())
    {
        return kCreateInequalityBoundsMap.at(aType)(aNumberOfConstraints);
    }
    else
    {
        throw utilities::Exception("Request for inequality bounds object on an equality constraint.");
    }
}

void add_linear_equality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint)
{
    aProblem.addLinearConstraint(
        aROLConstraint.mName, Teuchos::rcp(aROLConstraint.mConstraintFunction.release()),
        make_rol_vector(criteria::library::make_dual_vector(aROLConstraint.mNumberOfConstraints)));
}

void add_equality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint)
{
    aProblem.addConstraint(aROLConstraint.mName, Teuchos::rcp(aROLConstraint.mConstraintFunction.release()),
                           third_party_integration::rol::make_rol_vector(
                               criteria::library::make_dual_vector(aROLConstraint.mNumberOfConstraints)));
}

void add_linear_inequality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint)
{
    auto tInequalityBoundConstraint =
        create_inequality_bounds(aROLConstraint.mType, aROLConstraint.mNumberOfConstraints);
    auto tMultipliers = ROL::makePtr<std::vector<double>>(aROLConstraint.mNumberOfConstraints, 0);
    auto tMultipliersPtr = ROL::makePtr<ROL::StdVector<double>>(tMultipliers);
    constexpr bool tReset = false;

    aProblem.addLinearConstraint(
        aROLConstraint.mName, Teuchos::rcp(aROLConstraint.mConstraintFunction.release()),
        make_rol_vector(criteria::library::make_dual_vector(aROLConstraint.mNumberOfConstraints)),
        tInequalityBoundConstraint, tMultipliersPtr, tReset);
}

void add_inequality_constraint(ROL::Problem<double>& aProblem, ROLConstraint& aROLConstraint)
{
    auto tInequalityBoundConstraint =
        create_inequality_bounds(aROLConstraint.mType, aROLConstraint.mNumberOfConstraints);
    auto tMultipliers = ROL::makePtr<std::vector<double>>(aROLConstraint.mNumberOfConstraints, 0);
    auto tMultipliersPtr = ROL::makePtr<ROL::StdVector<double>>(tMultipliers);
    constexpr bool tReset = false;
    aProblem.addConstraint(aROLConstraint.mName, Teuchos::rcp(aROLConstraint.mConstraintFunction.release()),
                           third_party_integration::rol::make_rol_vector(
                               criteria::library::make_dual_vector(aROLConstraint.mNumberOfConstraints)),
                           tInequalityBoundConstraint, tMultipliersPtr, tReset);
}

auto determine_constraint_combination(const ROLConstraint& aConstraint) -> ConstraintCombination
{
    if (aConstraint.mType == criteria::library::ConstraintType::kEquality)
    {
        if (aConstraint.mLinear)
        {
            return ConstraintCombination::kLinearEquality;
        }
        return ConstraintCombination::kNonlinearEquality;
    }
    else
    {
        if (aConstraint.mLinear)
        {
            return ConstraintCombination::kLinearInequality;
        }
        return ConstraintCombination::kNonlinearInequality;
    }
}

}  // namespace detail
}  // namespace plato::third_party_integration::rol
