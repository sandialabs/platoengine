#ifndef PLATO_FILTER_LIBRARY_FILTERREGISTRATION
#define PLATO_FILTER_LIBRARY_FILTERREGISTRATION

#include <filesystem>
#include <memory>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/StateCache.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::filter::library
{
struct FilterParameters;
class FilterInterface;
}  // namespace plato::filter::library

namespace plato::filter::library
{

/// A `std::variant` with alternatives corresponding to input blocks
/// created using the PLATO_FILTER_INPUT_BLOCK_STRUCT macro.
using FilterInput = core::InputVariant<input_parser::ParsedInput, input_parser::IsFilterInput>;
using ValidatedFilterInput = core::ValidatedInputTypeWrapper<
    core::ValidatedInputVariant<input_parser::ParsedInput, input_parser::IsFilterInput>>;

using FilterFunction = core::Function<
    const analysis::AnalysisDomainMesh&,
    core::FunctionInfo<analysis::AnalysisDomainMesh, core::evaluation::kFunction>,
    core::FunctionInfo<FilterJacobian, core::evaluation::kFirstDerivative>,
    core::FunctionInfo<FilterAdjointJacobian, core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>>;

using FilterRegistration = core::FactoryRegistration<FilterFunction, ValidatedFilterInput>;
using FilterCache =
    plato::utilities::StateCache<std::shared_ptr<library::FilterInterface>, const analysis::AnalysisDomainMesh&>;

/// @brief Loads a filter from a shared library.
/// @param aInput The input parameters defining the filter's properties.
/// @param aSharedLibraryPath The path at which the shared library is located.
[[nodiscard]] std::unique_ptr<FilterInterface> load_filter(const FilterParameters& aParams,
                                                           const std::filesystem::path& aSharedLibraryPath);

[[nodiscard]] bool is_filter_function_registered(std::string_view aFunctionName);

/// @brief Returns @a FilterFunction that uses a @a FilterCache to reconstruct the filter object if the mesh has
/// changed.
/// @param aCacheFunction a callable that constructs the specified @a FilterCache object.
template <typename CacheFunction>
FilterFunction make_filter_function_from_cache(const CacheFunction& aCacheFunction);

template <typename CacheFunction>
FilterFunction make_filter_function_from_cache(const CacheFunction& aCacheFunction)
{
    auto tFilterCache = aCacheFunction();
    return FilterFunction{
        [tFilterCache](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) mutable
        { return tFilterCache.compute(aAnalysisDomainMesh)->filter(aAnalysisDomainMesh); },
        [tFilterCache](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) mutable
        { return make_filter_jacobian(tFilterCache.compute(aAnalysisDomainMesh), aAnalysisDomainMesh); },
        [tFilterCache](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) mutable
        { return make_filter_adjoint_jacobian(tFilterCache.compute(aAnalysisDomainMesh), aAnalysisDomainMesh); }};
}
}  // namespace plato::filter::library

#endif
