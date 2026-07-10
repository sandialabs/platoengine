#ifndef PLATO_SERVICES_SHAREDLIBRARYSETUPTEARDOWN
#define PLATO_SERVICES_SHAREDLIBRARYSETUPTEARDOWN

#include <dlfcn.h>

#include <filesystem>
#include <optional>
#include <string_view>

#include "plato/utilities/Exception.hpp"

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::services
{
/// @brief Contains a path to a shared library, as well as optional function names for function that will be called when
/// loading and unloading the library.
struct SharedLibraryData
{
    std::filesystem::path mSharedLibraryPath;
    std::optional<std::string> mLibraryLoadFunctionName = std::nullopt;
    std::optional<std::string> mLibraryShutdownFunctionName = std::nullopt;
};

/// @brief An RAII-style object for loading functions from a shared library.
class [[nodiscard]] SharedLibrarySetupTeardown
{
   public:
    /// @throw plato::utilities::Exception On error loading the shared library.
    explicit SharedLibrarySetupTeardown(SharedLibraryData aSharedLibraryData);

    /// @throw plato::utilities::Exception On error loading the shared library.
    explicit SharedLibrarySetupTeardown(std::filesystem::path aSharedLibraryPath);

    ~SharedLibrarySetupTeardown();

    SharedLibrarySetupTeardown(const SharedLibrarySetupTeardown&) = delete;
    auto operator=(const SharedLibrarySetupTeardown&) -> SharedLibrarySetupTeardown& = delete;
    SharedLibrarySetupTeardown(SharedLibrarySetupTeardown&& /*aSharedLibrary*/) noexcept;
    auto operator=(SharedLibrarySetupTeardown&& /*aSharedLibrary*/) noexcept -> SharedLibrarySetupTeardown&;

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
    [[nodiscard]] auto loadFunction(std::string_view aFunctionName);

    SharedLibraryData mSharedLibraryData;
    void* mSharedLibrary = nullptr;
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
                                   mSharedLibraryData.mSharedLibraryPath.string() +
                                   "\ndlsym error: " + std::string{tErrorMessage}};
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
