#ifndef PLATO_THIRDPARTYINTEGRATION_ROL_PARAMETERBUILDER
#define PLATO_THIRDPARTYINTEGRATION_ROL_PARAMETERBUILDER

#include <Teuchos_VerbosityLevel.hpp>
#include <optional>
#include <string>
#include <vector>

namespace plato::third_party_integration::rol
{

///@brief A struct to hold the drop down list mSublistNames into a Teuchos Parameter List, the parameter name
/// mParameterName and its value mValue
template <typename T>
struct ParameterAndValue
{
    std::vector<std::string> mSublistNames;
    std::string mParameterName;
    T mValue;
};

///@brief Make a ParameterAndValue for the iteration limit setting in ROL
auto make_status_test_iteration_limit(const std::optional<int> aValue = std::nullopt) -> ParameterAndValue<int>;

///@brief Make a ParameterAndValue for the gradient tolerance setting in ROL
auto make_status_test_gradient_tolerance(const std::optional<double> aValue = std::nullopt)
    -> ParameterAndValue<double>;

///@brief Make a ParameterAndValue for the step tolerance setting in ROL
auto make_status_test_step_tolerance(const std::optional<double> aValue = std::nullopt) -> ParameterAndValue<double>;

///@brief Make a ParameterAndValue for the step tolerance setting in ROL
auto make_general_output_level(const std::optional<int> aValue = std::nullopt) -> ParameterAndValue<int>;

///@brief Make a ParameterAndValue for the step composite step output level setting in ROL
auto make_step_composite_step_output_level(const std::optional<int> aValue = std::nullopt) -> ParameterAndValue<int>;

///@brief Make a ParameterAndValue for the trust region initial radius setting in ROL
auto make_trust_region_initial_radius(const std::optional<double> aValue = std::nullopt) -> ParameterAndValue<double>;

///@brief Make a ParameterAndValue for the general inexact hessiang flag setting in ROL
auto make_general_inexact_hessian(const std::optional<bool> aValue = std::nullopt) -> ParameterAndValue<bool>;

///@brief Make a ParameterAndValue for the secant use as hessian flag setting in ROL
auto make_secant_use_as_hessian(const std::optional<bool> aValue = std::nullopt) -> ParameterAndValue<bool>;

}  // namespace plato::third_party_integration::rol

#endif
