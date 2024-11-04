#include "plato/third_party_integration/rol/ParameterBuilder.hpp"

#include <string_view>

namespace plato::third_party_integration::rol
{

namespace
{

const auto kVerboseDefault = int{1};
const auto kCompositeStepVerboseDefault = int{0};

constexpr std::string_view kStatusTest = "Status Test";
constexpr std::string_view kGeneral = "General";
constexpr std::string_view kStep = "Step";
constexpr std::string_view kCompositeStep = "Composite Step";
constexpr std::string_view kTrustRegion = "Trust Region";
constexpr std::string_view kSecant = "Secant";

constexpr std::string_view kIterationLimit = "Iteration Limit";
constexpr std::string_view kGradientTolerance = "Gradient Tolerance";
constexpr std::string_view kStepTolerance = "Step Tolerance";
constexpr std::string_view kOutputLevel = "Output Level";
constexpr std::string_view kInitialRadius = "Initial Radius";
constexpr std::string_view kInexactHessianTimesAVector = "Inexact Hessian-Times-A-Vector";
constexpr std::string_view kUseAsHessian = "Use as Hessian";

}  // namespace

auto make_status_test_iteration_limit(const std::optional<int> aValue) -> ParameterAndValue<int>
{
    return ParameterAndValue<int>{{std::string{kStatusTest}}, std::string{kIterationLimit}, aValue.value_or(1)};
}

auto make_status_test_gradient_tolerance(const std::optional<double> aValue) -> ParameterAndValue<double>
{
    return ParameterAndValue<double>{
        {std::string{kStatusTest}}, std::string{kGradientTolerance}, aValue.value_or(1.e-12)};
}

auto make_status_test_step_tolerance(const std::optional<double> aValue) -> ParameterAndValue<double>
{
    return ParameterAndValue<double>{{std::string{kStatusTest}}, std::string{kStepTolerance}, aValue.value_or(1.e-14)};
}

auto make_general_output_level(const std::optional<int> aValue) -> ParameterAndValue<int>
{
    return ParameterAndValue<int>{{std::string{kGeneral}}, std::string{kOutputLevel}, aValue.value_or(kVerboseDefault)};
}

auto make_step_composite_step_output_level(const std::optional<int> aValue) -> ParameterAndValue<int>
{
    return ParameterAndValue<int>{{std::string{kStep}, std::string{kCompositeStep}},
                                  std::string{kOutputLevel},
                                  aValue.value_or(kCompositeStepVerboseDefault)};
}

auto make_trust_region_initial_radius(const std::optional<double> aValue) -> ParameterAndValue<double>
{
    return ParameterAndValue<double>{
        {std::string{kStep}, std::string{kTrustRegion}}, std::string{kInitialRadius}, aValue.value_or(15)};
}

auto make_general_inexact_hessian(const std::optional<bool> aValue) -> ParameterAndValue<bool>
{
    return ParameterAndValue<bool>{
        {std::string{kGeneral}}, std::string{kInexactHessianTimesAVector}, aValue.value_or(false)};
}

auto make_secant_use_as_hessian(const std::optional<bool> aValue) -> ParameterAndValue<bool>
{
    return ParameterAndValue<bool>{
        {std::string{kGeneral}, std::string{kSecant}}, std::string{kUseAsHessian}, aValue.value_or(false)};
}

}  // namespace plato::third_party_integration::rol
