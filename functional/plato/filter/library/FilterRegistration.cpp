#include "plato/filter/library/FilterRegistration.hpp"

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterSharedLibraryDecorator.hpp"
#include "plato/services/SharedLibraryObject.hpp"

namespace plato::filter::library
{
std::unique_ptr<FilterInterface> load_filter(const FilterParameters& aParams,
                                             const std::filesystem::path& aSharedLibraryPath)
{
    using FilterFunctionSignature = std::unique_ptr<FilterInterface>(const FilterParameters&);
    return std::make_unique<FilterSharedLibraryDecorator>(services::make_shared_library_object<FilterFunctionSignature>(
        aSharedLibraryPath, kCreateFilterFunctionName, aParams));
}

auto is_new_filter_function_registered(const std::string_view aFunctionName) -> bool
{
    return core::is_factory_function_registered<FilterFunction, NewValidatedFilterInput>(aFunctionName);
}

}  // namespace plato::filter::library
