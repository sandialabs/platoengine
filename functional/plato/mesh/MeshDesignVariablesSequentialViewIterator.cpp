#include "plato/mesh/MeshDesignVariablesSequentialViewIterator.hpp"

namespace plato::mesh
{
template <typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator++()
    -> MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>&
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
auto MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator*() const
    -> const reference
{
    return detail::dereferenced_proxy(mCurrentIterators, mEndIterators);
}

template <typename InnerIteratorType, typename IteratorCategory>
bool MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator==(
    const MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
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
bool MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>::operator!=(
    const MeshDesignVariablesSequentialViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
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

template struct MeshDesignVariablesSequentialViewIterator<MeshDesignVariables::ScalarFieldVector::iterator,
                                                          std::output_iterator_tag>;
template struct MeshDesignVariablesSequentialViewIterator<MeshDesignVariables::ScalarFieldVector::const_iterator,
                                                          std::input_iterator_tag>;

}  // namespace plato::mesh
