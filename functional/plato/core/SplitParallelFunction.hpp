#ifndef PLATO_CORE_SPLITPARALLELFUNCTION
#define PLATO_CORE_SPLITPARALLELFUNCTION

#include <boost/mpi/communicator.hpp>

#include "plato/core/Function.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::core
{
/// @brief Requirements for a function that generates a parallel Function.
template <typename F>
concept ParallelMakeFunction =
    requires(const F& aF) { requires kIsFunction<decltype(aF(boost::mpi::communicator{}))>; };

/// @brief Creates a parallelized function in which all ranks on @a aParentCommunicator participate, but only @a
/// aNumberOfProcessors actually do any computation. The remaining ranks block.
/// @pre @a aNumberOfProcessors must be less than or equal to the number of ranks on @a aParentCommunicator.
/// @pre @a aNumberOfProcessors must be greater than 0.
[[nodiscard]] auto make_split_parallel_function(const boost::mpi::communicator& aParentCommunicator,
                                                const std::size_t aNumberOfProcessors,
                                                const ParallelMakeFunction auto& aMakeFunction)
{
    assert(aNumberOfProcessors > std::size_t{0});
    assert(aNumberOfProcessors <= static_cast<std::size_t>(aParentCommunicator.size()));

    using FunctionType = std::invoke_result_t<decltype(aMakeFunction), boost::mpi::communicator>;
    using FunctionInfoTuple = typename FunctionType::TupleHelper::InfoTuple;

    const auto tFunctionTuple = [=]<std::size_t... kIndices>(std::index_sequence<kIndices...>)
    {
        const auto tMPIGroupCommunicator =
            utilities::group_split_communicator(aParentCommunicator, aNumberOfProcessors);

        return std::make_tuple(
            [=, mLocalFunction = aMakeFunction(tMPIGroupCommunicator.mSplitCommunicator)](auto&& aDomain)
            {
                constexpr auto tIndex = kIndices;
                using Info = std::tuple_element_t<tIndex, FunctionInfoTuple>;

                constexpr auto tParentCommunicatorBroadcastRank = 0;

                const auto tEvaluateFunction =
                    [&mLocalFunction](const boost::mpi::communicator&, auto&& aDomainForEvaluation)
                {
                    return mLocalFunction.template evaluate<Info::order, Info::ordering>(
                        std::forward<decltype(aDomainForEvaluation)>(aDomainForEvaluation));
                };

                return utilities::compute_on_root_group(tMPIGroupCommunicator, tParentCommunicatorBroadcastRank,
                                                        tEvaluateFunction, std::forward<decltype(aDomain)>(aDomain));
            }...);
    }(std::make_index_sequence<std::tuple_size_v<FunctionInfoTuple>>());

    return std::make_from_tuple<FunctionType>(tFunctionTuple);
}
}  // namespace plato::core

#endif
