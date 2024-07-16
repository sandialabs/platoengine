#include "plato/criteria/library/CriterionRegistration.hpp"

#include <boost/mpi/communicator.hpp>
#include <iterator>
#include <set>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/services/AppConfiguration.hpp"

namespace plato::criteria::library
{
namespace
{
std::string criterion_registration_name(const std::string_view aAppName, const std::string_view aCriterionName)
{
    return std::string{aAppName} + ":" + std::string{aCriterionName};
}
}  // namespace

bool is_criterion_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<CriterionFunction, CriterionInput>(aFunctionName);
}

bool is_parallel_criterion_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<CriterionFunction, CriterionInput, boost::mpi::communicator>(
        aFunctionName);
}

std::string criterion_registration_name(const services::AppConfiguration& aAppConfiguration,
                                        const services::CriterionConfiguration& aCriterionConfiguration)
{
    return criterion_registration_name(aAppConfiguration.mName, aCriterionConfiguration.mName);
}

std::string criterion_registration_name(const boost::optional<input_parser::AppName>& aAppName,
                                        const input_parser::CriterionName& aCriterionName)
{
    return criterion_registration_name(aAppName ? aAppName->mToken : input_parser::kBuiltinAppName,
                                       aCriterionName.mToken);
}

std::string builtin_criterion_registration_name(const std::string_view aCriterionName)
{
    return criterion_registration_name(input_parser::kBuiltinAppName, aCriterionName);
}

std::set<std::string> registered_criteria_names()
{
    auto tAllCriteria = std::set<std::string>{};
    auto tSerialFunctions = core::registered_function_names<CriterionFunction, CriterionInput>();
    std::move(tSerialFunctions.begin(), tSerialFunctions.end(), std::inserter(tAllCriteria, tAllCriteria.begin()));
    auto tParallelFunctions =
        core::registered_function_names<CriterionFunction, CriterionInput, boost::mpi::communicator>();
    std::move(tParallelFunctions.begin(), tParallelFunctions.end(), std::inserter(tAllCriteria, tAllCriteria.begin()));

    return tAllCriteria;
}

}  // namespace plato::criteria::library
