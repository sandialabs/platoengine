#ifndef PLATO_FILTER_LIBRARY_FILTERFACTORY
#define PLATO_FILTER_LIBRARY_FILTERFACTORY

#include <memory>

#include "plato/core/Function.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::filter::library
{
/// @brief Creates a Function used for filtering a nodal density field for density TO.
/// @param aInput The input parameters parsed from an input deck.
[[nodiscard]] FilterFunction make_filter_function(const ValidatedFilterInput& aInput);
}  // namespace plato::filter::library

#endif
