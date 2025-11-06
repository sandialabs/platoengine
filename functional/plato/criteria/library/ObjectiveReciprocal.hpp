#ifndef PLATO_CRITERIA_LIBRARY_OBJECTIVERECIPROCAL
#define PLATO_CRITERIA_LIBRARY_OBJECTIVERECIPROCAL

#include <cmath>
#include <limits>
#include <stdexcept>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"

namespace plato::criteria::library
{
/// @brief Returns a core::Function that reciprocates an objective (given f, returns 1/f)
[[nodiscard]] inline auto reciprocal_function();

/// @brief If @a aObjectiveGoal is kMinimizeReciprocal, composes the core::Function @a aFunction with a function that
/// returns the reciprocal. Else, returns @a aFunction.
template <typename F>
[[nodiscard]] auto make_reciprocal_criterion_function(F aFunction, const ObjectiveGoal& aObjectiveGoal);

[[nodiscard]] inline auto reciprocal_function()
{
    return core::make_function_with_first_derivative(
        [](const double aArg)
        {
            if (std::fabs(aArg) < std::numeric_limits<double>::min())
            {
                throw std::invalid_argument("Reciprocal function cannot be called with a value of 0.");
            }
            else
            {
                return 1.0 / aArg;
            }
        },
        [](const double aArg)
        {
            if (std::fabs(aArg) < std::numeric_limits<double>::min())
            {
                throw std::invalid_argument("Reciprocal function cannot be called with a value of 0.");
            }
            else
            {
                return -1.0 / aArg / aArg;
            }
        });
}

template <typename F>
[[nodiscard]] auto make_reciprocal_criterion_function(F aFunction, const ObjectiveGoal& aObjectiveGoal)
{
    return aObjectiveGoal == ObjectiveGoal::kMinimizeReciprocal
               ? core::compose(reciprocal_function(), std::move(aFunction))
               : std::move(aFunction);
}

}  // namespace plato::criteria::library

#endif
