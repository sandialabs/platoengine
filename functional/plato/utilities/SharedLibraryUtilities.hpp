#ifndef PLATO_UTILITIES_SHAREDLIBRARYUTILITIES
#define PLATO_UTILITIES_SHAREDLIBRARYUTILITIES

#include <dlfcn.h>

#include <filesystem>
#include <string_view>

#include "plato/utilities/Exception.hpp"

namespace plato::utilities
{
/// @throw plato::utilities::Exception On error loading the shared library.
[[nodiscard]] void* load_shared_library(const std::filesystem::path& aSharedLibPath);

/// @brief Loads a function from the shared library pointed to by @a aSharedLibrary.
/// @throw plato::utilities::Exception On error loading the shared function.
/// @pre @a aSharedLibrary must not be `nullptr`
template <typename FunctionType>
[[nodiscard]] FunctionType load_function(void* const aSharedLibrary,
                                         const std::string_view aFunctionName,
                                         const std::filesystem::path& aSharedLibPath)
{
    FunctionType tFunction = reinterpret_cast<FunctionType>(dlsym(aSharedLibrary, std::string{aFunctionName}.c_str()));
    if (tFunction == nullptr)
    {
        throw Exception{"Couldn't load function " + std::string{aFunctionName} + " from shared lib at " +
                        aSharedLibPath.string()};
    }
    return tFunction;
}
}  // namespace plato::utilities

#endif
