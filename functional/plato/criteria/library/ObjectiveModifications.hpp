#ifndef PLATO_CRITERIA_LIBRARY_OBJECTIVEMODIFICATIONS
#define PLATO_CRITERIA_LIBRARY_OBJECTIVEMODIFICATIONS

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"

namespace plato::criteria::library
{
/// @brief Returns a core::Function that reciprocates an objective (given f, returns 1/f)
[[nodiscard]] auto reciprocal_function();

/// @brief If @a aObjectiveGoal is kReciprocate, composes the core::Function @a aFunction with a function that returns
/// the reciprocal. Else, returns @a aFunction.
template <typename F>
[[nodiscard]] auto make_reciprocal_criterion_function(F aFunction, const ObjectiveGoal& aObjectiveGoal);

[[nodiscard]] auto reciprocal_function()
{
    return core::make_function_with_first_derivative([](const double aArg) { return 1.0 / aArg; },
                                                     [](const double aArg) { return -1.0 / aArg / aArg; });
}

template <typename F>
[[nodiscard]] auto make_reciprocal_criterion_function(F aFunction, const ObjectiveGoal& aObjectiveGoal)
{
    return aObjectiveGoal == ObjectiveGoal::kReciprocate ? core::compose(reciprocal_function(), std::move(aFunction))
                                                         : std::move(aFunction);
}

}  // namespace plato::criteria::library

#endif
