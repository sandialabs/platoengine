#include "plato/filter/library/FilterRegistration.hpp"

#include <optional>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::filter::library
{
std::unique_ptr<FilterInterface> load_filter(const FilterParameters& aParams,
                                             const std::filesystem::path& aSharedLibraryPath)
{
    using FilterFunctionSignature = std::unique_ptr<FilterInterface>(const FilterParameters&);

    auto tSharedLibrary = services::SharedLibrarySetupTeardown{aSharedLibraryPath};
    return tSharedLibrary.call<FilterFunctionSignature>(kCreateFilterFunctionName, aParams);
}

bool is_filter_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<FilterFunction, ValidatedFilterInput>(aFunctionName);
}

}  // namespace plato::filter::library
