#include "plato/filter/library/FilterRegistration.hpp"

#include <filesystem>
#include <memory>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterSharedLibraryDecorator.hpp"
#include "plato/services/SharedLibraryObject.hpp"

namespace plato::filter::library
{
auto load_filter(const FilterParameters& aParams, const std::filesystem::path& aSharedLibraryPath)
    -> std::unique_ptr<FilterInterface>
{
    using FilterFunctionSignature = std::unique_ptr<FilterInterface>(const FilterParameters&);
    return std::make_unique<FilterSharedLibraryDecorator>(services::make_shared_library_object<FilterFunctionSignature>(
        aSharedLibraryPath, kCreateFilterFunctionName, aParams));
}

auto is_filter_function_registered(const std::string_view aFunctionName) -> bool
{
    return core::is_factory_function_registered<FilterFunction, ValidatedFilterInput>(aFunctionName);
}

}  // namespace plato::filter::library
