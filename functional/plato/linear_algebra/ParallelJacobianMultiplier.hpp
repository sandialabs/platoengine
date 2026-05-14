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
                                                    const utilities::MPIGroupCommunicator& aMPIGroupCommunicator,
                                                    const int aParentBroadcastRank)
{
    if (aMPIGroupCommunicator.mParentCommunicator.size() == aMPIGroupCommunicator.mSplitCommunicator.size())
    {
        return aObject;
    }
    aObject.mVectorTimesJacobianFunction = [aMPIGroupCommunicator, aParentBroadcastRank,
                                            mVectorTimesJacobianFunction = aObject.mVectorTimesJacobianFunction](
                                               const linear_algebra::DynamicVector<double>& aRowVector)
    {
        auto tResult = linear_algebra::DynamicVector<double>{};
        if (aMPIGroupCommunicator.mGroupColor.mValue == 0)
        {
            tResult = mVectorTimesJacobianFunction(aRowVector);
        }
        return group_synchronized_object(tResult, aMPIGroupCommunicator, aParentBroadcastRank);
    };
    return aObject;
}

/// @brief Specialization of parallel_sync for AdjointJacobianMultiplier.
template <>
[[nodiscard]] inline auto group_synchronized_object(linear_algebra::AdjointJacobianMultiplier aObject,
                                                    const utilities::MPIGroupCommunicator& aMPIGroupCommunicator,
                                                    const int aParentBroadcastRank)
{
    return linear_algebra::AdjointJacobianMultiplier{
        group_synchronized_object(std::move(aObject.mValue), aMPIGroupCommunicator, aParentBroadcastRank)};
}

}  // namespace plato::utilities::customization

#endif
