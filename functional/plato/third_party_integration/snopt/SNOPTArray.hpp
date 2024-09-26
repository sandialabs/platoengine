#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTARRAY
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTARRAY

#include <memory>
#include <type_traits>

#include "plato/third_party_integration/snopt/NonNullPtr.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief A wrapper for C-style arrays used in SNOPT.
template <typename T>
class SNOPTArray
{
   public:
    SNOPTArray(T* const aArrayPtr, const std::size_t aSize);

    /// @brief The size of the array set on construction.
    [[nodiscard]] auto size() const -> std::size_t;

    /// @brief Non-const access to the entry in the array at index @a aIndex.
    /// @pre @a aIndex must be less than the size set on construction, enforced with an assertion.
    template <typename U = T>
    [[nodiscard]] auto operator[](const std::size_t aIndex) -> std::enable_if_t<!std::is_const_v<U>, T&>;

    /// @brief Const access to the entry in the array at index @a aIndex.
    /// @pre @a aIndex must be less than the size set on construction, enforced with an assertion.
    [[nodiscard]] auto operator[](const std::size_t aIndex) const -> const T&;

    /// @brief Const begin iterator
    [[nodiscard]] auto begin() const -> const T*;
    /// @brief Const end iterator
    [[nodiscard]] auto end() const -> const T*;

    /// @brief Non-const begin iterator
    template <typename U = T>
    [[nodiscard]] auto begin() -> std::enable_if_t<!std::is_const_v<U>, T*>;
    /// @brief Non-const end iterator
    template <typename U = T>
    [[nodiscard]] auto end() -> std::enable_if_t<!std::is_const_v<U>, T*>;

   private:
    NonNullPtr<T> mPtrToArray;
    std::size_t mSize;
};

template <typename T>
SNOPTArray<T>::SNOPTArray(T* const aArrayPtr, const std::size_t aSize) : mPtrToArray{aArrayPtr}, mSize{aSize}
{
}

template <typename T>
auto SNOPTArray<T>::size() const -> std::size_t
{
    return mSize;
}

template <typename T>
template <typename U>
auto SNOPTArray<T>::operator[](const std::size_t aIndex) -> std::enable_if_t<!std::is_const_v<U>, T&>
{
    assert(aIndex < mSize);
    return mPtrToArray.get()[aIndex];
}

template <typename T>
auto SNOPTArray<T>::operator[](const std::size_t aIndex) const -> const T&
{
    assert(aIndex < mSize);
    return mPtrToArray.get()[aIndex];
}

template <typename T>
auto SNOPTArray<T>::begin() const -> const T*
{
    return mPtrToArray.get();
}

template <typename T>
auto SNOPTArray<T>::end() const -> const T*
{
    return mPtrToArray.get() + mSize;
}

template <typename T>
template <typename U>
auto SNOPTArray<T>::begin() -> std::enable_if_t<!std::is_const_v<U>, T*>
{
    return mPtrToArray.get();
}

template <typename T>
template <typename U>
auto SNOPTArray<T>::end() -> std::enable_if_t<!std::is_const_v<U>, T*>
{
    return mPtrToArray.get() + mSize;
}
}  // namespace plato::third_party_integration::snopt

#endif
