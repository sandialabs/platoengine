#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_NONNULLPTR
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_NONNULLPTR

#include <cassert>
#include <type_traits>

namespace plato::third_party_integration::snopt
{
struct AssertionPolicy
{
    void operator()() { assert(false); }
};

/// @brief A wrapper for a pointer that guarantees the pointer is not `nullptr`
///
/// The guarantee is enforced with ErrorPolicy, and the default is an assertion.
template <typename T, typename ErrorPolicy = AssertionPolicy>
class NonNullPtr
{
   public:
    explicit NonNullPtr(T* aPtrToT);

    /// @brief Const access to the underlying object as a reference.
    [[nodiscard]] auto get() const -> const T*;

    /// @brief Non-const access to the underlying object as a reference.
    template <typename U = T>
    [[nodiscard]] auto get() -> std::enable_if_t<!std::is_const_v<U>, T*>;

   private:
    T* mPtrToT;
};

template <typename T, typename ErrorPolicy>
NonNullPtr<T, ErrorPolicy>::NonNullPtr(T* const aPtrToT) : mPtrToT{aPtrToT}
{
    if (mPtrToT == nullptr)
    {
        ErrorPolicy tErrorPolicy;
        tErrorPolicy();
    }
}

template <typename T, typename ErrorPolicy>
auto NonNullPtr<T, ErrorPolicy>::get() const -> const T*
{
    return mPtrToT;
}

template <typename T, typename ErrorPolicy>
template <typename U>
auto NonNullPtr<T, ErrorPolicy>::get() -> std::enable_if_t<!std::is_const_v<U>, T*>
{
    return mPtrToT;
}
}  // namespace plato::third_party_integration::snopt

#endif
