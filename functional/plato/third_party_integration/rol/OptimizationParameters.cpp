#include "plato/third_party_integration/rol/OptimizationParameters.hpp"

#include <ROL_LineSearchStep.hpp>
#include <ROL_StatusTest.hpp>

#include "plato/third_party_integration/rol/ParameterBuilder.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::rol
{
namespace
{

const auto kDefaultInputName = utilities::data_file_path("rol_inputs.xml");

}  // namespace

OptimizationParameters::OptimizationParameters(const std::optional<std::filesystem::path>& aInputFile)
    : mParameterList(detail::load_from_file(aInputFile.value_or(kDefaultInputName.value()))){};

void OptimizationParameters::verbose() { detail::make_verbose(mParameterList); }

void OptimizationParameters::approximateHessian() { detail::approximate_hessian(mParameterList); }

void OptimizationParameters::maximumIterations(const std::optional<unsigned int> aMaximumIterations)
{
    if (aMaximumIterations)
    {
        const int tMaximumIterations = static_cast<int>(aMaximumIterations.value());
        detail::set_parameter(mParameterList, make_status_test_iteration_limit(tMaximumIterations));
    }
}

void OptimizationParameters::gradientTolerance(const std::optional<double> aGradientTolerance)
{
    if (aGradientTolerance)
    {
        detail::set_parameter(mParameterList, make_status_test_gradient_tolerance(aGradientTolerance));
    }
}

void OptimizationParameters::stepTolerance(const std::optional<double> aStepTolerance)
{
    if (aStepTolerance)
    {
        detail::set_parameter(mParameterList, make_status_test_step_tolerance(aStepTolerance));
    }
}

void OptimizationParameters::initialSearchRadius(const std::optional<double> aInitialSearchRadius)
{
    if (aInitialSearchRadius)
    {
        detail::set_parameter(mParameterList, make_trust_region_initial_radius(aInitialSearchRadius));
    }
}

const ROL::ParameterList& OptimizationParameters::parameters() const { return mParameterList; }

void OptimizationParameters::writeParameters(const std::filesystem::path& aPath) const
{
    auto tParameters = mParameterList;
    ROL::writeParameterListToXmlFile(tParameters, aPath);
}

namespace detail
{

void make_verbose(ROL::ParameterList& aParameterList)
{
    detail::set_parameter(aParameterList, make_general_output_level(Teuchos::EVerbosityLevel::VERB_EXTREME));
    detail::set_parameter(aParameterList,
                          make_step_composite_step_output_level(Teuchos::EVerbosityLevel::VERB_EXTREME));
}

void approximate_hessian(ROL::ParameterList& aParameterList)
{
    detail::set_parameter(aParameterList, make_secant_use_as_hessian(true));
    detail::set_parameter(aParameterList, make_general_inexact_hessian(true));
}

ROL::ParameterList load_from_file(const std::filesystem::path& aInputFile)
{
    return *ROL::getParametersFromXmlFile(aInputFile);
}

auto retrieve_sublist_from_sequence(ROL::ParameterList& aParameterList, const std::vector<std::string>& aSequence)
    -> std::reference_wrapper<ROL::ParameterList>
{
    assert(!aSequence.empty());
    auto tCurrentSublistName = aSequence.begin();
    auto tCurrentSublist = std::ref(aParameterList);
    while (tCurrentSublistName != aSequence.end() && tCurrentSublist.get().isSublist(*tCurrentSublistName))
    {
        tCurrentSublist = std::ref(tCurrentSublist.get().sublist(*tCurrentSublistName));
        std::advance(tCurrentSublistName, 1);
    }

    return tCurrentSublist;
}

}  // namespace detail
}  // namespace plato::third_party_integration::rol
