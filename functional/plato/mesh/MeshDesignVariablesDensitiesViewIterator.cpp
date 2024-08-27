#include "plato/mesh/MeshDesignVariablesDensitiesViewIterator.hpp"

namespace plato::mesh
{
template <typename InnerIteratorType, typename IteratorCategory>
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator++()
    -> MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>&
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
auto MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator*() const -> const reference
{
    return detail::dereferenced_proxy(mCurrentIterators, mEndIterators);
}

template <typename InnerIteratorType, typename IteratorCategory>
bool MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator==(
    const MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
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
bool MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>::operator!=(
    const MeshDesignVariablesDensitiesViewIterator<InnerIteratorType, IteratorCategory>& aRHSIterator) const
{
    return !(*this == aRHSIterator);
}

namespace detail
{
auto min_id_iterator(const std::vector<std::optional<Density>>& aDensities) ->
    typename std::vector<std::optional<Density>>::const_iterator
{
    return std::min_element(aDensities.cbegin(), aDensities.cend(),
                            [](const auto& tLeftDensity, const auto& tRightDensity)
                            {
                                if (tLeftDensity && tRightDensity)
                                {
                                    return tLeftDensity->mGlobalMeshEntityID < tRightDensity->mGlobalMeshEntityID;
                                }
                                return tLeftDensity.has_value();
                            });
}

}  // namespace detail

template struct MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::DensityVector::iterator,
                                                         std::output_iterator_tag>;
template struct MeshDesignVariablesDensitiesViewIterator<MeshDesignVariables::DensityVector::const_iterator,
                                                         std::input_iterator_tag>;

}  // namespace plato::mesh
