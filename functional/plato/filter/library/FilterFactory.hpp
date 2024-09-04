#ifndef PLATO_FILTER_LIBRARY_FILTERFACTORY
#define PLATO_FILTER_LIBRARY_FILTERFACTORY

#include "plato/core/Function.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::design_variables
{
struct MeshDesignVariables;
}

namespace plato::filter::library
{
struct FilterJacobian;

using FilterFunction =
    core::Function<design_variables::MeshDesignVariables, FilterJacobian, const design_variables::MeshDesignVariables&>;

/// @brief Creates a Function used for filtering a nodal density field for density TO.
/// @param aInput The input parameters parsed from an input deck.
[[nodiscard]] FilterFunction make_filter_function(const ValidatedFilterInput& aInput);

}  // namespace plato::filter::library

#endif
