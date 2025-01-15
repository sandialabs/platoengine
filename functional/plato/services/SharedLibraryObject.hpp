#ifndef PLATO_SERVICES_SHAREDLIBRARYOBJECT
#define PLATO_SERVICES_SHAREDLIBRARYOBJECT

#include <filesystem>
#include <string_view>

#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::services
{
/// @brief The purpose of this class is to tie the lifetimes of a shared library managed by SharedLibSetupTeardown with
/// an object created from that library.
///
/// SharedLibSetupTeardown manages a shared library using RAII and so its lifetime must match that of an object created
/// from the shared library. This is useful when the main purpose of loading a shared library is to create a single
/// object, such as CriterionInterface or FilterInterface.
template <typename Object>
class SharedLibraryObject
{
   public:
    SharedLibraryObject(SharedLibrarySetupTeardown&& aSharedLibrary, Object&& aObject);

    [[nodiscard]] auto object() const -> const Object&;
    [[nodiscard]] auto object() -> Object&;

   private:
    SharedLibrarySetupTeardown mSharedLibrary;
    Object mObject;
};

/// @brief Creates a SharedLibrarySetupTeardown object and wraps the object created by calling
///  its call member in a SharedLibraryObject.
template <typename FunctionSignature, typename... Args>
auto make_shared_library_object(const std::filesystem::path& aSharedLibraryPath,
                                std::string_view aFunctionName,
                                Args&&... aArgs)
    -> SharedLibraryObject<std::invoke_result_t<FunctionSignature, Args...>>
{
    using Object = std::invoke_result_t<FunctionSignature, Args...>;
    auto tSharedLibrary = services::SharedLibrarySetupTeardown{aSharedLibraryPath};
    return SharedLibraryObject<Object>{
        std::move(tSharedLibrary), tSharedLibrary.call<FunctionSignature>(aFunctionName, std::forward<Args>(aArgs)...)};
}

template <typename Object>
SharedLibraryObject<Object>::SharedLibraryObject(SharedLibrarySetupTeardown&& aSharedLibrary, Object&& aObject)
    : mSharedLibrary{std::move(aSharedLibrary)}, mObject{std::move(aObject)}
{
}

template <typename Object>
auto SharedLibraryObject<Object>::object() const -> const Object&
{
    return mObject;
}

template <typename Object>
auto SharedLibraryObject<Object>::object() -> Object&
{
    return mObject;
}
}  // namespace plato::services

#endif
