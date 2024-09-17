#include "plato/analysis/AnalysisDomainMeshSequentialViewIterator.hpp"

namespace plato::analysis
{
template <typename InnerIteratorType, typename IteratorCategory>
auto AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator++()
    -> AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>&
{
    const auto tValues = detail::dereference_all(mCurrentIterators, mEndIterators);
    const auto tMinIDIterator = detail::min_id_iterator(tValues);
    if (tMinIDIterator != tValues.cend() && tMinIDIterator->has_value())
    {
        const auto tMinGlobalID = tMinIDIterator->value().mGlobalMeshEntityID;  // NOLINT
        for (const auto& [tCurrentIterator, tEndIterator] : utilities::Zip{mCurrentIterators, mEndIterators})
        {
            if (tCurrentIterator != tEndIterator && tCurrentIterator->mGlobalMeshEntityID == tMinGlobalID)
            {
                ++tCurrentIterator;
            }
        }
    }
    return *this;
}

template <typename InnerIteratorType, typename IteratorCategory>
auto AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator*() const -> const reference
{
    return detail::dereferenced_proxy(mCurrentIterators, mEndIterators);
}

template <typename InnerIteratorType, typename IteratorCategory>
bool AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator==(
    const AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    auto tAllEqual = true;
    for (const auto& [tLeftIterator, tRightIterator] :
         utilities::Zip{mCurrentIterators, aRHSIterator.mCurrentIterators})
    {
        tAllEqual &= tLeftIterator == tRightIterator;
    }
    return tAllEqual;
}

template <typename InnerIteratorType, typename IteratorCategory>
bool AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator!=(
    const AnalysisDomainMeshSequentialViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

namespace detail
{
auto min_id_iterator(const std::vector<std::optional<ScalarFieldValue>>& aScalarField) ->
    typename std::vector<std::optional<ScalarFieldValue>>::const_iterator
{
    return std::min_element(aScalarField.cbegin(), aScalarField.cend(),
                            [](const auto& tLeftValue, const auto& tRightValue)
                            {
                                if (tLeftValue && tRightValue)
                                {
                                    return tLeftValue->mGlobalMeshEntityID < tRightValue->mGlobalMeshEntityID;
                                }
                                return tLeftValue.has_value();
                            });
}

}  // namespace detail

template struct AnalysisDomainMeshSequentialViewIterator<AnalysisDomainMesh::ScalarFieldVector::iterator,
                                                         std::forward_iterator_tag>;
template struct AnalysisDomainMeshSequentialViewIterator<AnalysisDomainMesh::ScalarFieldVector::const_iterator,
                                                         std::input_iterator_tag>;

}  // namespace plato::analysis
