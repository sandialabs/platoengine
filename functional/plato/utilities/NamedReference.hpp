#ifndef PLATO_UTILITIES_NAMEDREFERENCE
#define PLATO_UTILITIES_NAMEDREFERENCE

#include <functional>

namespace plato::utilities
{
/// @brief A strongly-typed wrapper for clarifying interfaces that use references.
///
/// The purpose of this class is to facilitate using strong types for making function signatures clear or otherwise
/// discriminating types. NamedReference (as opposed to NamedType) is more appropriate for function arguments that are
/// passed by reference. The reference is stored in a `std::reference_wrapper` so that this object is easily copyable.
/// @sa NamedType
template <typename T, typename NameTag>
struct NamedReference
{
    constexpr explicit NamedReference(T& value) : mValue{std::ref(value)} {}

    std::reference_wrapper<T> mValue;
};

}  // namespace plato::utilities
#endif
