#include "plato/design_variables/MeshDesignVariablesRandomAccessView.hpp"

#include <numeric>

#include "plato/design_variables/MeshDesignVariablesSequentialView.hpp"

namespace plato::design_variables
{
namespace
{
std::optional<ScalarFieldValue> element_with_global_index(const MeshDesignVariables::ScalarFieldVector& aScalarField,
                                                          const ScalarFieldValue::IndexType aIndex)
{
    const auto tEntryToFind = ScalarFieldValue{aIndex, 0, 0.0};
    const auto tIter = std::lower_bound(aScalarField.cbegin(), aScalarField.cend(), tEntryToFind,
                                        [](const auto& aEntryLeft, const auto& aEntryRight)
                                        { return aEntryLeft.mGlobalMeshEntityID < aEntryRight.mGlobalMeshEntityID; });
    if (tIter != aScalarField.cend() && tIter->mGlobalMeshEntityID == aIndex)
    {
        return *tIter;
    }
    return std::nullopt;
}
}  // namespace

auto MeshDesignVariablesRandomAccessView::size() const -> std::size_t
{
    return MeshDesignVariablesSequentialView{mMeshDesignVariables.get()}.size();
}

auto MeshDesignVariablesRandomAccessView::operator[](const ScalarFieldValue::IndexType aIndex) const
    -> std::optional<ScalarFieldValue>
{
    for (const auto& tBlockScalarField : mMeshDesignVariables.get().mBlockScalarField)
    {
        if (const auto tResult = element_with_global_index(tBlockScalarField.second, aIndex))
        {
            return tResult;
        }
    }
    return std::nullopt;
}

}  // namespace plato::design_variables
