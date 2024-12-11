#ifndef PLATO_FILTER_LIBRARY_TESTUTILITIES_FILTERFUNCTION
#define PLATO_FILTER_LIBRARY_TESTUTILITIES_FILTERFUNCTION

#include <memory>

#include "plato/core/Function.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::filter::test_utilities
{
/// @brief Creates a FilterFunction from a FilterInterface.
[[nodiscard]] auto make_filter_function(const std::shared_ptr<library::FilterInterface>& aFilter)
    -> library::FilterFunction;
}  // namespace plato::filter::test_utilities

#endif
