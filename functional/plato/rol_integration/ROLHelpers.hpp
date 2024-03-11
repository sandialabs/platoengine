#ifndef PLATO_ROL_INTEGRATION_ROLHELPERS
#define PLATO_ROL_INTEGRATION_ROLHELPERS

#include <ROL_StdVector.hpp>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::rol_integration
{
/// @brief Converts @a aROLVector to a DynamicVector using a `dynamic_cast` to `ROL::StdVector`.
template <typename T>
[[nodiscard]] linear_algebra::DynamicVector<T> to_dynamic_vector(const ROL::Vector<T>& aROLVector);

/// @brief Converts @a aDynamicVector to a `ROL::StdVector`.
template <typename T>
[[nodiscard]] ROL::StdVector<T> to_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector);

/// @brief Converts @a aDynamicVector to a `ROL::Vector` managed pointer.
template <typename T>
[[nodiscard]] ROL::Ptr<ROL::Vector<T>> make_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector);

/// @brief Copies @a aVector into the storage of @a aROLVector using a `dynamic_cast` to `ROL::StdVector`.
template <typename T>
void assign_vector(ROL::Vector<T>& aROLVector, std::vector<T> aVector);

template <typename T>
linear_algebra::DynamicVector<T> to_dynamic_vector(const ROL::Vector<T>& aROLVector)
{
    const auto& tROLStdVector = dynamic_cast<const ROL::StdVector<T>&>(aROLVector);
    return linear_algebra::DynamicVector(*tROLStdVector.getVector());
}

template <typename T>
ROL::StdVector<T> to_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector)
{
    return ROL::StdVector<double>(ROL::makePtr<std::vector<T>>(std::move(aDynamicVector).stdVector()));
}

template <typename T>
ROL::Ptr<ROL::Vector<T>> make_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector)
{
    return ROL::makePtr<ROL::StdVector<double>>(to_rol_vector(std::move(aDynamicVector)));
}

template <typename T>
void assign_vector(ROL::Vector<T>& aROLVector, std::vector<T> aVector)
{
    auto& tROLStdVector = dynamic_cast<ROL::StdVector<T>&>(aROLVector);
    *tROLStdVector.getVector() = std::move(aVector);
}
}  // namespace plato::rol_integration

#endif
