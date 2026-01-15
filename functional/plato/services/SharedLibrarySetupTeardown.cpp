#include "plato/services/SharedLibrarySetupTeardown.hpp"

#include <dlfcn.h>

#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"

namespace plato::services
{
namespace
{
constexpr auto kUsingSanitizer = static_cast<bool>(BUILD_WITH_SANITIZER_FLAGS);

void* load_shared_library(const std::filesystem::path& aSharedLibPath)
{
    // RTLD_DEEPBIND is not compatible with building with sanitizer flags.
    // RTLD_DEEPBIND is required for external shared libraries to be opened correctly.
    // This is intended to enable sanitizer checks in the unit test suite during CI.
    constexpr auto tOptions = kUsingSanitizer ? RTLD_LAZY : (RTLD_LAZY | RTLD_DEEPBIND);
    void* const tSharedLibInterface = dlopen(aSharedLibPath.c_str(), tOptions);
    if (tSharedLibInterface == nullptr)
    {
        const auto tErrorMessage = std::string{dlerror()};
        throw utilities::Exception{"Couldn't load shared lib at " + aSharedLibPath.string() +
                                   ".\ndlopen error: " + tErrorMessage};
    }
    return tSharedLibInterface;
}

void close_if_nonnull(void* const aSharedLibrary)
{
    if (aSharedLibrary != nullptr)
    {
        dlclose(aSharedLibrary);
    }
}
}  // namespace

SharedLibrarySetupTeardown::SharedLibrarySetupTeardown(std::filesystem::path aSharedLibraryPath)
    : mSharedLibraryPath{std::move(aSharedLibraryPath)}, mSharedLibrary{load_shared_library(mSharedLibraryPath)}
{
}

SharedLibrarySetupTeardown::SharedLibrarySetupTeardown(SharedLibrarySetupTeardown&& aSharedLibrary) noexcept
    : mSharedLibraryPath{std::move(aSharedLibrary.mSharedLibraryPath)}, mSharedLibrary{aSharedLibrary.mSharedLibrary}
{
    aSharedLibrary.mSharedLibraryPath.clear();
    aSharedLibrary.mSharedLibrary = nullptr;
}

SharedLibrarySetupTeardown& SharedLibrarySetupTeardown::operator=(SharedLibrarySetupTeardown&& aSharedLibrary) noexcept
{
    if (this != &aSharedLibrary)
    {
        std::swap(mSharedLibrary, aSharedLibrary.mSharedLibrary);
        std::swap(mSharedLibraryPath, aSharedLibrary.mSharedLibraryPath);
    }
    return *this;
}

SharedLibrarySetupTeardown::~SharedLibrarySetupTeardown() { close_if_nonnull(mSharedLibrary); }

}  // namespace plato::services
