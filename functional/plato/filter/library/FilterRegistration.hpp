#ifndef PLATO_FILTER_LIBRARY_FILTERREGISTRATION
#define PLATO_FILTER_LIBRARY_FILTERREGISTRATION

#include <filesystem>
#include <memory>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::core
{
struct MeshProxy;
}

namespace plato::filter::library
{
struct FilterParameters;
}

namespace plato::filter::library
{
class FilterInterface;
struct FilterJacobian;

/// A `std::variant` with alternatives corresponding to input blocks
/// created using the PLATO_FILTER_INPUT_BLOCK_STRUCT macro.
using FilterInput = core::InputVariant<input_parser::ParsedInput, input_parser::IsFilterInput>;
using ValidatedFilterInput = core::ValidatedInputTypeWrapper<
    core::ValidatedInputVariant<input_parser::ParsedInput, input_parser::IsFilterInput>>;
using FilterFunction = core::Function<core::MeshProxy, FilterJacobian, const core::MeshProxy&>;
using FilterRegistration = core::FactoryRegistration<FilterFunction, ValidatedFilterInput>;

/// @brief Loads a filter from a shared library.
/// @param aInput The input parameters defining the filter's properties.
/// @param aSharedLibraryPath The path at which the shared library is located.
[[nodiscard]] std::unique_ptr<FilterInterface> load_filter(const FilterParameters& aParams,
                                                           const std::filesystem::path& aSharedLibraryPath);

[[nodiscard]] bool is_filter_function_registered(std::string_view aFunctionName);
}  // namespace plato::filter::library

#endif
