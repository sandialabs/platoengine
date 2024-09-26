#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_OBJECTIVECONSTRAINTARRAYVIEW
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_OBJECTIVECONSTRAINTARRAYVIEW

#include <cassert>
#include <type_traits>

#include "plato/third_party_integration/snopt/SNOPTArray.hpp"
#include "plato/third_party_integration/snopt/ViewSizeTypes.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief A view into an array that splits the array into a single objective and multiple constraints.
///
/// This is used to fill the `F` array passed by SNOPT.
template <typename T>
class ObjectiveConstraintArrayView
{
   public:
    /// @brief Construction from a pointer to an array and a size.
    ObjectiveConstraintArrayView(T* aArrayPtr, ConstraintSizeType aNumberOfConstraints);

    /// @brief Const access to the objective, whose index is give by objectiveArrayIndex.
    [[nodiscard]] auto objective() const -> const T&;

    /// @brief Mutable access to the objective, whose index is give by objectiveArrayIndex.
    template <typename U = T>
    [[nodiscard]] auto objective() -> std::enable_if_t<!std::is_const_v<U>, T&>;

    /// @brief Returns a const view into the partition of the array representing constraints
    [[nodiscard]] auto constraints() const -> SNOPTArray<const T>;

    /// @brief Returns a view into the partition of the array representing constraints
    template <typename U = T>
    [[nodiscard]] auto constraints() -> std::enable_if_t<!std::is_const_v<T>, SNOPTArray<T>>;

    /// @brief The index used to store the objective.
    [[nodiscard]] static constexpr auto objectiveArrayIndex() -> std::size_t;

   private:
    [[nodiscard]] auto numberOfConstraints() const -> std::size_t;

    SNOPTArray<T> mArray;

    static constexpr std::size_t mObjectiveIndex = 0U;
    static constexpr std::size_t mNumberOfObjectives = 1U;
};

template <typename T>
ObjectiveConstraintArrayView<T>::ObjectiveConstraintArrayView(T* const aArrayPtr,
                                                              const ConstraintSizeType aNumberOfConstraints)
    : mArray{aArrayPtr, aNumberOfConstraints.mValue + mNumberOfObjectives}
{
}

template <typename T>
auto ObjectiveConstraintArrayView<T>::objective() const -> const T&
{
    return mArray[mObjectiveIndex];
}

template <typename T>
template <typename U>
auto ObjectiveConstraintArrayView<T>::objective() -> std::enable_if_t<!std::is_const_v<U>, T&>
{
    return mArray[mObjectiveIndex];
}

template <typename T>
auto ObjectiveConstraintArrayView<T>::constraints() const -> SNOPTArray<const T>
{
    return SNOPTArray<const T>{mArray.begin() + mNumberOfObjectives, numberOfConstraints()};
}

template <typename T>
template <typename U>
auto ObjectiveConstraintArrayView<T>::constraints() -> std::enable_if_t<!std::is_const_v<T>, SNOPTArray<T>>
{
    return SNOPTArray<T>{mArray.begin() + mNumberOfObjectives, numberOfConstraints()};
}

template <typename T>
auto ObjectiveConstraintArrayView<T>::numberOfConstraints() const -> std::size_t
{
    return mArray.size() - 1U;
}

template <typename T>
constexpr auto ObjectiveConstraintArrayView<T>::objectiveArrayIndex() -> std::size_t
{
    return mObjectiveIndex;
}

}  // namespace plato::third_party_integration::snopt

#endif
