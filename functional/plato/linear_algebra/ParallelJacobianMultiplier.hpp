#ifndef PLATO_LINEAR_ALGEBRA_PARALLELJACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_PARALLELJACOBIANMULTIPLIER

#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::utilities::customization
{
/// @brief Specialization of parallel_sync for JacobianMultiplier.
///
/// This will return an object that synchronizes the result of the JacobianMultiplier function evaluations after they
/// are evaluated. Otherwise, there is no way to communicate the `std::function` over MPI.
/// @note This must be in the `plato::core` namespace.
template <>
[[nodiscard]] inline auto group_synchronized_object(linear_algebra::JacobianMultiplier aObject,
                                                    const utilities::MPIGroupCommunicator& aGroupCommunicator,
                                                    const int aParentBroadcastRank)
{
    if (aGroupCommunicator.mParentCommunicator.size() == aGroupCommunicator.mSplitCommunicator.size())
    {
        return aObject;
    }
    aObject.mVectorTimesJacobianFunction = [aGroupCommunicator, aParentBroadcastRank,
                                            tMVectorTimesJacobianFunction = aObject.mVectorTimesJacobianFunction](
                                               const linear_algebra::DynamicVector<double>& aRowVector)
    {
        auto tResult = linear_algebra::DynamicVector<double>{};
        if (aGroupCommunicator.mGroupColor.mValue == 0)
        {
            tResult = tMVectorTimesJacobianFunction(aRowVector);
        }
        return group_synchronized_object(tResult, aGroupCommunicator, aParentBroadcastRank);
    };
    return aObject;
}

/// @brief Specialization of parallel_sync for AdjointJacobianMultiplier.
template <>
[[nodiscard]] inline auto group_synchronized_object(linear_algebra::AdjointJacobianMultiplier aObject,
                                                    const utilities::MPIGroupCommunicator& aGroupCommunicator,
                                                    const int aParentBroadcastRank)
{
    return linear_algebra::AdjointJacobianMultiplier{
        group_synchronized_object(std::move(aObject.mValue), aGroupCommunicator, aParentBroadcastRank)};
}

}  // namespace plato::utilities::customization

#endif
