#ifndef PLATO_THIRDPARTYINTEGRATION_ROL_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_ROL_UTILITIES

#include <ROL_Ptr.hpp>
#include <ROL_StdVector.hpp>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::third_party_integration::rol
{

/// @brief Converts @a aROLVector to a DynamicVector using a `dynamic_cast` to `ROL::StdVector`.
template <typename T>
[[nodiscard]] auto to_dynamic_vector(const ROL::Vector<T>& aROLVector) -> linear_algebra::DynamicVector<T>;

/// @brief Converts @a aDynamicVector to a `ROL::StdVector`.
template <typename T>
[[nodiscard]] auto to_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector) -> ROL::StdVector<T>;

/// @brief Converts @a aDynamicVector to a `ROL::Vector` managed pointer.
template <typename T>
[[nodiscard]] auto make_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector) -> ROL::Ptr<ROL::StdVector<T>>;

/// @brief Copies @a aVector into the storage of @a aROLVector using a `dynamic_cast` to `ROL::StdVector`.
template <typename T>
void assign_vector(ROL::Vector<T>& aROLVector, std::vector<T> aVector);

/// @brief Helper function that creates a random direction for use in diagnostic checks.
///
/// @post The returned vector has dimension @a aDimension and magnitude 1.
[[nodiscard]] auto generate_perturbation(int aDimension) -> ROL::StdVector<double>;

/// @brief Makes @a aROLVector entries all equal to @a aValue.
void make_uniform(ROL::Vector<double>& aROLVector, double aValue);

/// @brief Randomizes @a aROLVector, but scales the result to have unit norm.
void randomize_and_normalize(ROL::Vector<double>& aROLVector);

template <typename T>
auto to_dynamic_vector(const ROL::Vector<T>& aROLVector) -> linear_algebra::DynamicVector<T>
{
    const auto& tROLStdVector = dynamic_cast<const ROL::StdVector<T>&>(aROLVector);
    return linear_algebra::DynamicVector(*tROLStdVector.getVector());
}

template <typename T>
auto to_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector) -> ROL::StdVector<T>
{
    return ROL::StdVector<double>(ROL::makePtr<std::vector<T>>(std::move(aDynamicVector).stdVector()));
}

template <typename T>
auto make_rol_vector(linear_algebra::DynamicVector<T> aDynamicVector) -> ROL::Ptr<ROL::StdVector<T>>
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
