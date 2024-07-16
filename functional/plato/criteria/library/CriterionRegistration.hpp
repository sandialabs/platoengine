#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION

#include <boost/mpi/communicator.hpp>
#include <boost/optional.hpp>
#include <set>
#include <string_view>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/services/AppConfiguration.hpp"

namespace plato::core
{
struct MeshProxy;
}  // namespace plato::core

namespace plato::criteria::library
{
struct CriterionInput
{
    unsigned int mNumberOfProcessors;
    input_parser::FileList mInputFiles;
};

using CriterionFunction = core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;
using CriterionRegistration = core::FactoryRegistration<CriterionFunction, CriterionInput>;
using ParallelCriterionRegistration =
    core::FactoryRegistration<CriterionFunction, CriterionInput, boost::mpi::communicator>;

[[nodiscard]] bool is_criterion_function_registered(const std::string_view aFunctionName);
[[nodiscard]] bool is_parallel_criterion_function_registered(const std::string_view aFunctionName);

/// @brief Creates a name from @a aAppConfiguration and @a aCriterionConfiguration that can be used
/// to uniquely register a criterion function.
///
/// This overload is for registering plugins.
[[nodiscard]] std::string criterion_registration_name(const services::AppConfiguration& aAppConfiguration,
                                                      const services::CriterionConfiguration& aCriterionConfiguration);

/// @brief Creates a name from @a aAppName and @a aCriterionName that can be used
/// to find a registered a criterion function.
///
/// This overload is for finding a registered app/criterion using user input.
[[nodiscard]] std::string criterion_registration_name(const boost::optional<input_parser::AppName>& aAppName,
                                                      const input_parser::CriterionName& aCriterionName);

/// @brief Creates a unique name from @a aCriterionName that can be used uniquely register a built-in criterion.
[[nodiscard]] std::string builtin_criterion_registration_name(std::string_view aCriterionName);

/// @brief Returns the full list of registered criteria, useful for error messages.
[[nodiscard]] std::set<std::string> registered_criteria_names();

}  // namespace plato::criteria::library

#endif
