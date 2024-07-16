#ifndef PLATO_SERVICES_SHAREDLIBRARYSETUPTEARDOWN
#define PLATO_SERVICES_SHAREDLIBRARYSETUPTEARDOWN

#include <dlfcn.h>

#include <filesystem>
#include <string_view>

#include "plato/services/AppConfiguration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::services
{
/// @brief An RAII-style object for loading functions from a shared library.
class [[nodiscard]] SharedLibrarySetupTeardown
{
   public:
    /// @throw plato::utilities::Exception On error loading the shared library.
    explicit SharedLibrarySetupTeardown(std::filesystem::path aSharedLibraryPath);

    /// @todo Figure out how to call `dlclose`, though a library that is loaded will likely be needed for the remainder
    /// of the application run. The deleters of the std::unique_ptrs passed out of the library use the glibc `free`
    /// linked with that shared lib. A crash could result if the library is closed. The solution may be to make this a
    /// singleton and keep the libraries open until program exit. Or use weak_ptr for the objects returned and only
    /// close a library when all references to the objects returned from a shared lib are gone.
    ~SharedLibrarySetupTeardown() = default;

    SharedLibrarySetupTeardown(const SharedLibrarySetupTeardown&) = delete;
    SharedLibrarySetupTeardown& operator=(const SharedLibrarySetupTeardown&) = delete;
    SharedLibrarySetupTeardown(SharedLibrarySetupTeardown&&) = default;
    SharedLibrarySetupTeardown& operator=(SharedLibrarySetupTeardown&&) = default;

    /// @brief Calls function with name @a aFunction name and arguments @a aArgs from the shared library loaded on
    /// construction.
    /// @tparam FunctionSignature The signature of the function that will be loaded from the shared library. This must
    /// match the signature of the actual function.
    /// @throw plato::utilities::Exception On error loading the shared library function.
    template <typename FunctionSignature, typename... Args>
    auto call(std::string_view aFunctionName, Args&&... aArgs);

   private:
    /// @brief Loads a function from the shared library pointed to by @a aSharedLibrary.
    /// @tparam FunctionSignature The signature of the function that will be loaded from the shared library. This must
    /// match the signature of the actual function.
    /// @throw plato::utilities::Exception On error loading the shared library function.
    template <typename FunctionSignature>
    [[nodiscard]] auto loadFunction(const std::string_view aFunctionName);

   private:
    std::filesystem::path mSharedLibraryPath;
    void* mSharedLibrary;
};

template <typename FunctionSignature>
[[nodiscard]] auto SharedLibrarySetupTeardown::loadFunction(const std::string_view aFunctionName)
{
    using FunctionPtr = std::add_pointer_t<FunctionSignature>;
    auto tFunction =
        reinterpret_cast<FunctionPtr>(dlsym(mSharedLibrary, std::string{aFunctionName}.c_str()));  // NOLINT
    if (tFunction == nullptr)
    {
        char* const tErrorMessage = dlerror();
        throw utilities::Exception{"Couldn't load function " + std::string{aFunctionName} + " from shared lib at " +
                                   mSharedLibraryPath.string() + "\ndlsym error: " + std::string{tErrorMessage}};
    }
    return tFunction;
}

template <typename FunctionSignature, typename... Args>
auto SharedLibrarySetupTeardown::call(const std::string_view aFunctionName, Args&&... aArgs)
{
    const auto tFunction = loadFunction<FunctionSignature>(aFunctionName);
    return tFunction(std::forward<Args>(aArgs)...);
}
}  // namespace plato::services

#endif
