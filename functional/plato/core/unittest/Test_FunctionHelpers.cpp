#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/core/unittest/TestHelpers.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{
TEST(FunctionHelpers, IsOnlyMember)
{
    static_assert(!core::detail::is_only_member<int>());
    static_assert(core::detail::is_only_member<int, int>());
    static_assert(core::detail::is_only_member<int, const int&>());
    static_assert(!core::detail::is_only_member<int, int, int>());
}

TEST(FunctionHelpers, IsCopyable)
{
    using EvaluateInfo = FunctionInfo<double, evaluation::kFunction>;
    using FirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using SecondDerivativeInfo = FunctionInfo<double, evaluation::kSecondDerivative>;
    using FirstDerivativeAdjointInfo = FunctionInfo<double, evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>;

    using DomainType1 = double;
    using DomainType2 = int;
    // Copyable
    static_assert(
        core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo>, DomainType1, std::tuple<EvaluateInfo>>());
    static_assert(core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, FirstDerivativeInfo>, DomainType1,
                                            std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo>, DomainType1,
                                            std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(
        core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, FirstDerivativeInfo>, DomainType1,
                                  std::tuple<EvaluateInfo, FirstDerivativeInfo, FirstDerivativeAdjointInfo>>());

    // Not copyable
    static_assert(
        !core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo>, DomainType2, std::tuple<EvaluateInfo>>());
    static_assert(!core::detail::is_copyable<DomainType1, std::tuple<SecondDerivativeInfo>, DomainType1,
                                             std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(!core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, SecondDerivativeInfo>, DomainType1,
                                             std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(!core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, SecondDerivativeInfo>, DomainType1,
                                             std::tuple<EvaluateInfo>>());
}

}  // namespace plato::core::unittest
