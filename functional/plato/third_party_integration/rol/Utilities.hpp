#ifndef PLATO_THIRDPARTYINTEGRATION_ROL_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_ROL_UTILITIES

#include <ROL_StdVector.hpp>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::third_party_integration::rol
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

/// @brief Helper function that creates a random direction for use in diagnostic checks.
///
/// @post The returned vector has dimension @a aDimension and magnitude 1.
[[nodiscard]] ROL::StdVector<double> generate_perturbation(const int aDimension);

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

}  // namespace plato::third_party_integration::rol

#endif
