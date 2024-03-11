#ifndef PLATO_FILTER_LIBRARY_FILTERREGISTRATION
#define PLATO_FILTER_LIBRARY_FILTERREGISTRATION

#include <filesystem>
#include <memory>

#include "plato/core/FactoryRegistration.hpp"
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
class FilterInterface;
struct FilterJacobian;

using FilterFunction = core::Function<core::MeshProxy, FilterJacobian, const core::MeshProxy&>;
using FilterInput = input_parser::density_topology;
using FilterRegistration = core::FactoryRegistration<FilterFunction, FilterInput>;

[[nodiscard]] auto make_filter_function_from_interface(std::unique_ptr<FilterInterface> aFilter) -> FilterFunction;

/// @brief Loads a filter from a shared library.
/// @param aInput The input parameters defining the filter's properties.
/// @param aSharedLibraryPath The path at which the shared library is located.
[[nodiscard]] std::unique_ptr<FilterInterface> load_filter(const input_parser::density_topology& aInput,
                                                           const std::filesystem::path& aSharedLibraryPath);

[[nodiscard]] bool is_filter_function_registered(std::string_view aFunctionName);
}  // namespace plato::filter::library

#endif
