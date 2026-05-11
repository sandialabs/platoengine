#ifndef PLATO_LINEAR_ALGEBRA_PARALLELJACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_PARALLELJACOBIANMULTIPLIER

#include "plato/core/ParallelSync.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::core
{
/// @brief Specialization of parallel_sync for JacobianMultiplier.
///
/// This will return an object that synchronizes the result of the JacobianMultiplier function evaluations after they
/// are evaluated. Otherwise, there is no way to communicate the `std::function` over MPI.
/// @note This must be in the `plato::core` namespace.
template <>
[[nodiscard]] inline auto parallel_sync(linear_algebra::JacobianMultiplier aObject,
                                        const core::ParentCommunicator& aParentCommunicator,
                                        const core::SplitCommunicator& aSplitCommunicator,
                                        const ParentBroadcastRank aParentBroadcastRank,
                                        const GroupColor aGroupColor)
{
    if (aParentCommunicator.mValue.size() == aSplitCommunicator.mValue.size())
    {
        return aObject;
    }
    aObject.mVectorTimesJacobianFunction = [aParentCommunicator, aSplitCommunicator, aParentBroadcastRank, aGroupColor,
                                            mVectorTimesJacobianFunction = aObject.mVectorTimesJacobianFunction](
                                               const linear_algebra::DynamicVector<double>& aRowVector)
    {
        auto tResult = linear_algebra::DynamicVector<double>{};
        if (aGroupColor.mValue == 0)
        {
            tResult = mVectorTimesJacobianFunction(aRowVector);
        }
        return parallel_sync(tResult, aParentCommunicator, aSplitCommunicator, aParentBroadcastRank, aGroupColor);
    };
    return aObject;
}

/// @brief Specialization of parallel_sync for AdjointJacobianMultiplier.
template <>
[[nodiscard]] inline auto parallel_sync(linear_algebra::AdjointJacobianMultiplier aObject,
                                        const core::ParentCommunicator& aParentCommunicator,
                                        const core::SplitCommunicator& aSplitCommunicator,
                                        const ParentBroadcastRank aParentBroadcastRank,
                                        const GroupColor aGroupColor)
{
    return linear_algebra::AdjointJacobianMultiplier{parallel_sync(
        std::move(aObject.mValue), aParentCommunicator, aSplitCommunicator, aParentBroadcastRank, aGroupColor)};
}

}  // namespace plato::core

#endif
