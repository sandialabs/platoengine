#ifndef PLATO_FILTER_LIBRARY_FILTERFACTORY
#define PLATO_FILTER_LIBRARY_FILTERFACTORY

#include "plato/core/Function.hpp"

namespace plato::input_parser
{
struct density_topology;
}

namespace plato::core
{
struct MeshProxy;
}

namespace plato::filter::library
{
struct FilterJacobian;

using FilterFunction = core::Function<core::MeshProxy, FilterJacobian, const core::MeshProxy&>;

/// @brief Creates a Function used for filtering a nodal density field for density TO.
/// @param aInput The input parameters parsed from an input deck.
[[nodiscard]] FilterFunction make_filter_function(const input_parser::density_topology& aInput);

}  // namespace plato::filter::library

#endif
