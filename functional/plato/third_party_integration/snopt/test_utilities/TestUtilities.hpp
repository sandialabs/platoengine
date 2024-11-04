#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_TEST_UTILITIES_TESTUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_TEST_UTILITIES_TESTUTILITIES

#include <gtest/gtest.h>

#include <memory>
#include <numeric>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/snopt/SparseMatrixBuilder.hpp"

namespace plato::third_party_integration::snopt::test_utilities
{
template <typename IndexType, typename EntryType>
using Triple = std::tuple<typename SparseMatrixBuilder<IndexType, EntryType>::Row,
                          typename SparseMatrixBuilder<IndexType, EntryType>::Column,
                          EntryType>;

/// @brief Utility function for converting a scalar to a vector to simplify creating test functions
[[nodiscard]] inline auto vector_from_scalar(const double aScalar) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{aScalar};
}

/// @brief Utility function for converting a DynamicVector to a JacobianMultiplier to simplify creating test functions
[[nodiscard]] inline auto jacobian_from_gradient(const linear_algebra::DynamicVector<double>& aGradient)
    -> linear_algebra::JacobianMultiplier
{
    return linear_algebra::JacobianMultiplier{[aGradient](const linear_algebra::DynamicVector<double>& aVector)
                                              { return aVector[0] * aGradient; }};
}

/// @brief Creates a dynamic array with size @a aSize, and fills it using `iota` starting at 0.
template <typename T>
auto array_with_iota(const std::size_t aSize) -> std::unique_ptr<T[]>;

/// @brief Checks each entry in @a aLeft and @a aRight for equality.
template <typename IndexType, typename EntryType>
void check_triple(const Triple<IndexType, EntryType>& aLeft,
                  const Triple<IndexType, EntryType>& aRight,
                  const plato::test_utilities::TestContext& aTestContext);

template <typename T>
auto array_with_iota(const std::size_t aSize) -> std::unique_ptr<T[]>
{
    auto tArray = std::make_unique<int[]>(aSize);
    std::iota(tArray.get(), tArray.get() + aSize, 0);
    return tArray;
}

template <typename IndexType, typename EntryType>
void check_triple(const Triple<IndexType, EntryType>& aLeft,
                  const Triple<IndexType, EntryType>& aRight,
                  const plato::test_utilities::TestContext& aTestContext)
{
    using SparseMatrix = SparseMatrixBuilder<IndexType, EntryType>;
    EXPECT_EQ(std::get<typename SparseMatrix::Row>(aLeft).mValue, std::get<typename SparseMatrix::Row>(aRight).mValue)
        << aTestContext;
    EXPECT_EQ(std::get<typename SparseMatrix::Column>(aLeft).mValue,
              std::get<typename SparseMatrix::Column>(aRight).mValue)
        << aTestContext;
    EXPECT_EQ(std::get<double>(aLeft), std::get<double>(aRight)) << aTestContext;
}

}  // namespace plato::third_party_integration::snopt::test_utilities
#endif
