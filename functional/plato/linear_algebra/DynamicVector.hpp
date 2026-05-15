#ifndef PLATO_LINEAR_ALGEBRA_DYNAMICVECTOR
#define PLATO_LINEAR_ALGEBRA_DYNAMICVECTOR

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <vector>

namespace plato::linear_algebra
{

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

    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto operator[](std::size_t aIndex) const -> const T&;

    [[nodiscard]] auto dot(const DynamicVector<T>& aRightOperand) const -> T;
    [[nodiscard]] auto stdVector() const& -> const std::vector<T>&;
    [[nodiscard]] auto stdVector() && -> std::vector<T>;

    /// @pre The size of @a aRightOperand must be equal to this object's size. Checked with an assertion.
    auto operator+=(const DynamicVector<T>& aRightOperand) -> DynamicVector<T>&;
    auto operator*=(const T aScalar) -> DynamicVector<T>&;
    [[nodiscard]] auto operator==(const DynamicVector<T>& aRight) const -> bool;

    template <typename Archive, typename U>
    friend void serialize(Archive& aArchive, DynamicVector<U>& aVector, const unsigned int /*aVersion*/);

   private:
    std::vector<T> mVector;
};

/// @brief Computes the L2 norm of @a aVector.
template <typename T>
    requires std::convertible_to<T, double>
[[nodiscard]] auto norm(const DynamicVector<T>& aVector) -> double;

template <typename T>
[[nodiscard]] auto operator+(DynamicVector<T>&& aLeft, const DynamicVector<T>& aRight) -> DynamicVector<T>
{
    aLeft += aRight;
    return std::move(aLeft);
}

template <typename T>
[[nodiscard]] auto operator+(const DynamicVector<T>& aLeft, DynamicVector<T>&& aRight) -> DynamicVector<T>
{
    aRight += aLeft;
    return std::move(aRight);
}

// NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
// This overload only exists to prevent ambiguous overloads when two r-values are used to call this operator.
template <typename T>
[[nodiscard]] auto operator+(DynamicVector<T>&& aLeft, DynamicVector<T>&& aRight) -> DynamicVector<T>
{
    aLeft += aRight;
    return std::move(aLeft);
}
// NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)

template <typename T>
[[nodiscard]] auto operator+(const DynamicVector<T>& aLeft, const DynamicVector<T>& aRight) -> DynamicVector<T>
{
    DynamicVector<T> tCopy = aLeft;
    tCopy += aRight;
    return tCopy;
}

template <typename T>
[[nodiscard]] auto operator*(DynamicVector<T> aLeft, const T& aRight) -> DynamicVector<T>
{
    aLeft *= aRight;
    return aLeft;
}

template <typename T>
[[nodiscard]] auto operator*(const T& aLeft, DynamicVector<T> aRight) -> DynamicVector<T>
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
auto DynamicVector<T>::size() const -> std::size_t
{
    return mVector.size();
}

template <typename T>
auto DynamicVector<T>::operator[](std::size_t aIndex) const -> const T&
{
    return mVector[aIndex];
}

template <typename T>
auto DynamicVector<T>::stdVector() const& -> const std::vector<T>&
{
    return mVector;
}

template <typename T>
auto DynamicVector<T>::stdVector() && -> std::vector<T>
{
    return std::move(mVector);
}

template <typename T>
auto DynamicVector<T>::dot(const DynamicVector<T>& aRightOperand) const -> T
{
    assert(size() == aRightOperand.size());
    return std::inner_product(std::begin(mVector), std::end(mVector), std::begin(aRightOperand.mVector), T{});
}

template <typename T>
auto DynamicVector<T>::operator+=(const DynamicVector<T>& aRightOperand) -> DynamicVector<T>&
{
    assert(size() == aRightOperand.size());
    std::transform(std::begin(mVector), std::end(mVector), std::begin(aRightOperand.mVector), std::begin(mVector),
                   [](const T& aLeft, const T& aRight) { return aLeft + aRight; });
    return *this;
}

template <typename T>
auto DynamicVector<T>::operator*=(const T aScalar) -> DynamicVector<T>&
{
    std::transform(std::begin(mVector), std::end(mVector), std::begin(mVector),
                   [&aScalar](const T& aValue) { return aScalar * aValue; });
    return *this;
}

template <typename T>
[[nodiscard]] auto DynamicVector<T>::operator==(const DynamicVector<T>& aRight) const -> bool
{
    return mVector == aRight.mVector;
}

template <typename T>
    requires std::convertible_to<T, double>
auto norm(const DynamicVector<T>& aVector) -> double
{
    const auto tSumOfSquares = std::transform_reduce(aVector.stdVector().begin(), aVector.stdVector().end(), 0.0,
                                                     std::plus{}, [](const double aEntry) { return aEntry * aEntry; });
    return std::sqrt(tSumOfSquares);
}

}  // namespace plato::linear_algebra

#endif
