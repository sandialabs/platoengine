#ifndef PLATO_SERVICES_SHAREDLIBRARYOBJECT
#define PLATO_SERVICES_SHAREDLIBRARYOBJECT

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
