#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_VECTORWITHMINIMUMSIZE
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_VECTORWITHMINIMUMSIZE

#include <type_traits>
#include <vector>

namespace plato::third_party_integration::snopt
{
/// @brief The purpose of this class is to provide a vector that may have 0 size, but a valid data pointer.
///
/// The SNOPT interface requires pointer arguments for several arrays that may be empty; however, for those arguments,
/// the documentation states that the arrays should have memory allocated to store at least one value.
/// This provides a minimal interface with the underlying vector that is needed in the SNOPT interfaces.
///
/// @tparam T Must be default-constructible.
template <typename T>
class VectorWithValidDataPtr
{
    static_assert(std::is_default_constructible_v<T>,
                  "VectorWithValidDataPtr can only be instantiated with types that are default constructible.");

   public:
    /// @brief Default construction results in a vector with size 0, but a valid data pointer.
    VectorWithValidDataPtr() = default;

    /// @brief Construction by setting the initial size, and default initializing all entries.
    /// @param aInitialSize If @a aInitialSize is 0, this is the same as default construction.
    VectorWithValidDataPtr(std::size_t aInitialSize);

    /// @brief Appends an entry @a aU the vector as with `vector::push_back`.
    template <typename U>
    void pushBack(U&& aU);

    /// @brief The size of the vector.
    auto size() const -> std::size_t;

    /// @brief Const accessor for the vector's entries.
    /// @param aIndex must be less than the vector's size.
    auto operator[](std::size_t aIndex) const -> const T&;

    /// @brief Non-const Accessor for the vector's entries.
    /// @param aIndex must be less than the vector's size.
    auto operator[](std::size_t aIndex) -> T&;

    /// @brief Pointer to the first element as with `vector::data`.
    /// Guaranteed to be valid even if size is 0.
    auto data() const -> const T*;

    /// @brief Pointer to the first element as with `vector::data`.
    /// Guaranteed to be valid even if size is 0.
    auto data() -> T*;

   private:
    bool mIsInitialized = false;
    std::vector<T> mVector{T{}};
};

template <typename T>
VectorWithValidDataPtr<T>::VectorWithValidDataPtr(const std::size_t aInitialSize)
    : mIsInitialized{aInitialSize != 0U}, mVector(aInitialSize)
{
    if (aInitialSize == 0U)
    {
        mVector.emplace_back();
    }
}

template <typename T>
template <typename U>
void VectorWithValidDataPtr<T>::pushBack(U&& aU)
{
    if (!mIsInitialized)
    {
        mVector.front() = std::forward<U>(aU);
        mIsInitialized = true;
    }
    else
    {
        mVector.push_back(std::forward<U>(aU));
    }
}

template <typename T>
auto VectorWithValidDataPtr<T>::size() const -> std::size_t
{
    return mIsInitialized ? mVector.size() : std::size_t{0};
}

template <typename T>
auto VectorWithValidDataPtr<T>::data() const -> const T*
{
    return mVector.data();
}

template <typename T>
auto VectorWithValidDataPtr<T>::data() -> T*
{
    return mVector.data();
}

template <typename T>
auto VectorWithValidDataPtr<T>::operator[](std::size_t aIndex) const -> const T&
{
    return mVector[aIndex];
}

template <typename T>
auto VectorWithValidDataPtr<T>::operator[](std::size_t aIndex) -> T&
{
    return mVector[aIndex];
}

}  // namespace plato::third_party_integration::snopt

#endif
