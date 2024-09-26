#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_OBJECTIVECONSTRAINTGRADIENTARRAYVIEW
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_OBJECTIVECONSTRAINTGRADIENTARRAYVIEW

#include <type_traits>

#include "plato/third_party_integration/snopt/SNOPTArray.hpp"
#include "plato/third_party_integration/snopt/ViewSizeTypes.hpp"

namespace plato::third_party_integration::snopt
{

/// @brief An array view for objective and constraint gradients.
///
/// This is used to fill the `G` array passed by SNOPT.
template <typename T>
class ObjectiveConstraintGradientArrayView
{
   public:
    /// @brief Construction from a pointer to an array and a size.
    /// @pre @a aNumberOfDesignVariables must be greater than 0.
    ObjectiveConstraintGradientArrayView(T* aArrayPtr,
                                         ConstraintSizeType aNumberOfConstraints,
                                         DesignVariableSizeType aNumberOfDesignVariables);

    /// @brief Returns the number of constraints.
    [[nodiscard]] auto numberOfConstraints() const -> ConstraintSizeType;

    /// @brief Returns the number of design variables.
    [[nodiscard]] auto numberOfDesignVariables() const -> DesignVariableSizeType;

    /// @brief Returns a const view into the part of the underlying array associated with the objective gradient.
    [[nodiscard]] auto objectiveGradient() const -> SNOPTArray<const T>;

    /// @brief Returns a non-const view into the part of the underlying array associated with the objective gradient.
    template <typename U = T>
    [[nodiscard]] auto objectiveGradient() -> std::enable_if_t<!std::is_const_v<U>, SNOPTArray<T>>;

    /// @brief Returns a const view into the part of the underlying array associated with the constraint gradient with
    /// index @a aConstraintIndex.
    /// @pre @a aConstraintIndex must be less than numberOfConstraints.
    [[nodiscard]] auto constraintGradient(ConstraintSizeType aConstraintIndex) const -> SNOPTArray<const T>;

    /// @brief Returns a non-const view into the part of the underlying array associated with the constraint gradient
    /// with index @a aConstraintIndex.
    /// @pre @a aConstraintIndex must be less than numberOfConstraints.
    template <typename U = T>
    [[nodiscard]] auto constraintGradient(ConstraintSizeType aConstraintIndex)
        -> std::enable_if_t<!std::is_const_v<U>, SNOPTArray<T>>;

    /// @brief Returns the total array size `((1 + number of constraints) * number of design variables)`
    [[nodiscard]] auto arraySize() const -> std::size_t;

   private:
    [[nodiscard]] auto arrayIndex(ConstraintSizeType aConstraintIndex) const -> std::size_t;

   private:
    ConstraintSizeType mNumberOfConstraints;
    DesignVariableSizeType mNumberOfDesignVariables;

    SNOPTArray<T> mArray;

    static constexpr std::size_t mObjectiveIndex = 0U;
    static constexpr std::size_t mNumberOfObjectives = 1U;
};

template <typename T>
ObjectiveConstraintGradientArrayView<T>::ObjectiveConstraintGradientArrayView(
    T* const aArrayPtr,
    const ConstraintSizeType aNumberOfConstraints,
    const DesignVariableSizeType aNumberOfDesignVariables)
    : mNumberOfConstraints{aNumberOfConstraints},
      mNumberOfDesignVariables{aNumberOfDesignVariables},
      mArray{aArrayPtr, arraySize()}
{
    assert(mNumberOfDesignVariables.mValue > 0);
}

template <typename T>
auto ObjectiveConstraintGradientArrayView<T>::numberOfConstraints() const -> ConstraintSizeType
{
    return mNumberOfConstraints;
}

template <typename T>
auto ObjectiveConstraintGradientArrayView<T>::numberOfDesignVariables() const -> DesignVariableSizeType
{
    return mNumberOfDesignVariables;
}

template <typename T>
auto ObjectiveConstraintGradientArrayView<T>::objectiveGradient() const -> SNOPTArray<const T>
{
    return {mArray.begin(), mNumberOfDesignVariables.mValue};
}

template <typename T>
template <typename U>
auto ObjectiveConstraintGradientArrayView<T>::objectiveGradient()
    -> std::enable_if_t<!std::is_const_v<U>, SNOPTArray<T>>
{
    return {mArray.begin(), mNumberOfDesignVariables.mValue};
}

template <typename T>
auto ObjectiveConstraintGradientArrayView<T>::constraintGradient(const ConstraintSizeType aConstraintIndex) const
    -> SNOPTArray<const T>
{
    return {mArray.begin() + arrayIndex(aConstraintIndex), mNumberOfDesignVariables.mValue};
}

template <typename T>
template <typename U>
auto ObjectiveConstraintGradientArrayView<T>::constraintGradient(const ConstraintSizeType aConstraintIndex)
    -> std::enable_if_t<!std::is_const_v<U>, SNOPTArray<T>>
{
    return {mArray.begin() + arrayIndex(aConstraintIndex), mNumberOfDesignVariables.mValue};
}

template <typename T>
auto ObjectiveConstraintGradientArrayView<T>::arraySize() const -> std::size_t
{
    return (mNumberOfConstraints.mValue + mNumberOfObjectives) * mNumberOfDesignVariables.mValue;
}

template <typename T>
auto ObjectiveConstraintGradientArrayView<T>::arrayIndex(ConstraintSizeType aConstraintIndex) const -> std::size_t
{
    return (aConstraintIndex.mValue + mNumberOfObjectives) * mNumberOfDesignVariables.mValue;
}

}  // namespace plato::third_party_integration::snopt

#endif
