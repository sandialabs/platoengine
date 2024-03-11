#ifndef PLATO_LINEAR_ALGEBRA_DYNAMICVECTOR
#define PLATO_LINEAR_ALGEBRA_DYNAMICVECTOR

#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

namespace plato::linear_algebra
{
template <typename T>
class DynamicVector;

template <typename Archive, typename U>
void serialize(Archive& aArchive, DynamicVector<U>& aVector, const unsigned int aVersion);

/// @brief A wrapper for a `std::vector`, with functions for addition, multiplication and serialization.
///
/// The purpose of this class is to provide an interface with ROL vector types, isolating the ROL types
/// to the objective/constraint ROL classes.
/// Serialization is also implemented for communication using boost::mpi.
template <typename T>
class DynamicVector
{
   public:
    DynamicVector() = default;
    DynamicVector(std::size_t aSize, const T& aInitialValue);
    explicit DynamicVector(std::vector<T> aVector);
    explicit DynamicVector(std::initializer_list<T> aList);

    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] const T& operator[](std::size_t aIndex) const;

    [[nodiscard]] T dot(const DynamicVector<T>& aRightOperand) const;
    [[nodiscard]] const std::vector<T>& stdVector() const&;
    [[nodiscard]] std::vector<T> stdVector() &&;

    /// @pre The size of @a aRightOperand must be equal to this object's size. Checked with an assertion.
    DynamicVector<T>& operator+=(const DynamicVector<T>& aRightOperand);
    DynamicVector<T>& operator*=(const T aScalar);
    [[nodiscard]] bool operator==(const DynamicVector<T>& aRight) const;

    template <typename Archive, typename U>
    friend void serialize(Archive& aArchive, DynamicVector<U>& aVector, const unsigned int aVersion);

   private:
    std::vector<T> mVector;
};

template <typename T>
[[nodiscard]] DynamicVector<T> operator+(DynamicVector<T>&& aLeft, const DynamicVector<T>& aRight)
{
    aLeft += aRight;
    return std::move(aLeft);
}

template <typename T>
[[nodiscard]] DynamicVector<T> operator+(const DynamicVector<T>& aLeft, DynamicVector<T>&& aRight)
{
    aRight += aLeft;
    return std::move(aRight);
}

template <typename T>
[[nodiscard]] DynamicVector<T> operator+(DynamicVector<T>&& aLeft, DynamicVector<T>&& aRight)
{
    aLeft += aRight;
    return std::move(aLeft);
}

template <typename T>
[[nodiscard]] DynamicVector<T> operator+(const DynamicVector<T>& aLeft, const DynamicVector<T>& aRight)
{
    DynamicVector<T> tCopy = aLeft;
    tCopy += aRight;
    return tCopy;
}

template <typename T>
[[nodiscard]] DynamicVector<T> operator*(DynamicVector<T> aLeft, const T& aRight)
{
    aLeft *= aRight;
    return aLeft;
}

template <typename T>
[[nodiscard]] DynamicVector<T> operator*(const T& aLeft, DynamicVector<T> aRight)
{
    aRight *= aLeft;
    return aRight;
}

template <typename T>
DynamicVector<T>::DynamicVector(std::size_t aSize, const T& aInitialValue) : mVector(aSize, aInitialValue)
{
}

template <typename T>
DynamicVector<T>::DynamicVector(std::vector<T> aVector) : mVector(std::move(aVector))
{
}

template <typename T>
DynamicVector<T>::DynamicVector(std::initializer_list<T> aList) : mVector(std::move(aList))
{
}

template <typename T>
std::size_t DynamicVector<T>::size() const
{
    return mVector.size();
}

template <typename T>
const T& DynamicVector<T>::operator[](std::size_t aIndex) const
{
    return mVector[aIndex];
}

template <typename T>
const std::vector<T>& DynamicVector<T>::stdVector() const&
{
    return mVector;
}

template <typename T>
std::vector<T> DynamicVector<T>::stdVector() &&
{
    return std::move(mVector);
}

template <typename T>
T DynamicVector<T>::dot(const DynamicVector<T>& aRightOperand) const
{
    return std::inner_product(std::begin(mVector), std::end(mVector), std::begin(aRightOperand.mVector), T{});
}

template <typename T>
DynamicVector<T>& DynamicVector<T>::operator+=(const DynamicVector<T>& aRightOperand)
{
    assert(size() == aRightOperand.size());
    std::transform(std::begin(mVector), std::end(mVector), std::begin(aRightOperand.mVector), std::begin(mVector),
                   [](const T& aLeft, const T& aRight) { return aLeft + aRight; });
    return *this;
}

template <typename T>
DynamicVector<T>& DynamicVector<T>::operator*=(const T aScalar)
{
    std::transform(std::begin(mVector), std::end(mVector), std::begin(mVector),
                   [&aScalar](const T& aValue) { return aScalar * aValue; });
    return *this;
}

template <typename T>
[[nodiscard]] bool DynamicVector<T>::operator==(const DynamicVector<T>& aRight) const
{
    return mVector == aRight.mVector;
}

}  // namespace plato::linear_algebra

#endif
