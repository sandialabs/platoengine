#include "plato/utilities/SharedLibraryUtilities.hpp"

namespace plato::utilities
{
void* load_shared_library(const std::filesystem::path& aSharedLibPath)
{
    void* const tSharedLibInterface = dlopen(aSharedLibPath.c_str(), RTLD_LAZY | RTLD_DEEPBIND);
    if (tSharedLibInterface == nullptr)
    {
        char* const tErrorMessage = dlerror();
        throw Exception{"Couldn't load shared lib at " + aSharedLibPath.string() + ". Error: " + std::string{tErrorMessage}};
    }
    return tSharedLibInterface;
}
}  // namespace plato::utilities