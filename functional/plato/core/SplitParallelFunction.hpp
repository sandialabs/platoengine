#ifndef PLATO_CORE_SPLITPARALLELFUNCTION
#define PLATO_CORE_SPLITPARALLELFUNCTION

#include <boost/mpi/communicator.hpp>

#include "plato/core/Function.hpp"
#include "plato/utilities/RankSplitVector.hpp"

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
                                                const ParallelMakeFunction auto& aMakeFunction);

namespace detail
{
/// @brief Evaluates @a aFunction and communicates the result to a blocking group, whose root is @a aBlockingGroupRoot
/// on @a aParentCommunicator.
/// @note If @a aBlockingGroupRoot is empty, then the parent communicator was not split and no communication is
/// necessary.
/// @return The result of the function evaluation.
template <std::size_t kOrder, core::MatrixOrdering kOrdering, int kMPITag, typename Domain, typename... Info>
[[nodiscard]] auto function_evaluation_and_sync(const core::Function<Domain, Info...>& aFunction,
                                                Domain&& aDomain,
                                                const boost::mpi::communicator& aParentCommunicator,
                                                const std::optional<int> aBlockingGroupRoot)
{
    auto tResult = aFunction.template evaluate<kOrder, kOrdering>(std::forward<Domain>(aDomain));
    if (aParentCommunicator.rank() == 0U && aBlockingGroupRoot)
    {
        aParentCommunicator.send(aBlockingGroupRoot.value(), kMPITag, tResult);
    }
    return tResult;
}

using ParentCommunicator = utilities::NamedType<boost::mpi::communicator, struct ParentCommunicatorTag>;
using SplitCommunicator = utilities::NamedType<boost::mpi::communicator, struct SplitCommunicatorTag>;

/// @brief Receives a result from @a aParentCommunicator and then synchronizes that with all ranks in @a
/// aSplitCommunicator.
template <int kMPITag, typename Info>
[[nodiscard]] auto block_and_sync(const ParentCommunicator& aParentCommunicator,
                                  const SplitCommunicator& aSplitCommunicator)
{
    auto tResult = typename Info::Codomain{};
    if (aSplitCommunicator.mValue.rank() == 0)
    {
        aParentCommunicator.mValue.recv(0, kMPITag, tResult);
    }
    return broadcast_from_root(aSplitCommunicator.mValue, std::move(tResult));
}

}  // namespace detail

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
        const auto tGroupSizes =
            std::vector{static_cast<unsigned int>(aNumberOfProcessors),
                        static_cast<unsigned int>(aParentCommunicator.size() - aNumberOfProcessors)};
        const auto tGroupColor =
            utilities::rank_group_color(tGroupSizes, utilities::RankNamedType{aParentCommunicator.rank()});
        const auto tSplitCommunicator = aParentCommunicator.split(tGroupColor.mValue);

        return std::make_tuple(
            [=, mLocalFunction = aMakeFunction(tSplitCommunicator)](auto&& aDomain)
            {
                constexpr auto kIndex = kIndices;
                constexpr auto tMPITag = 42;
                using Info = std::tuple_element_t<kIndex, FunctionInfoTuple>;

                if (tGroupColor.mValue == 0)
                {
                    const auto tBlockingGroupRootOnParent = tGroupSizes.back() != 0U
                                                                ? std::make_optional(aNumberOfProcessors)
                                                                : std::optional<std::size_t>{};
                    return detail::function_evaluation_and_sync<Info::order, Info::ordering, tMPITag>(
                        mLocalFunction, std::forward<decltype(aDomain)>(aDomain), aParentCommunicator,
                        tBlockingGroupRootOnParent);
                }
                return detail::block_and_sync<tMPITag, Info>(detail::ParentCommunicator{aParentCommunicator},
                                                             detail::SplitCommunicator{tSplitCommunicator});
            }...);
    }(std::make_index_sequence<std::tuple_size_v<FunctionInfoTuple>>());

    return std::make_from_tuple<FunctionType>(tFunctionTuple);
}
}  // namespace plato::core

#endif
