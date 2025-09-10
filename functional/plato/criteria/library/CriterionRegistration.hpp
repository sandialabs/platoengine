#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION

#include <boost/mpi/communicator.hpp>
#include <boost/optional.hpp>
#include <set>
#include <string_view>
#include <utility>

#include "plato/components/ComponentType.hpp"
#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/services/AppConfiguration.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::criteria::library
{
struct CriterionInput
{
    unsigned int mNumberOfProcessors = 1U;
    input_parser::FileList mInputFiles;
    components::ComponentType mComponentType = components::ComponentType::kObjective;
    std::string mName;
};

using CriterionFunction =
    core::Function<const analysis::AnalysisDomainMesh&,
                   core::FunctionInfo<double, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

using VectorCriterionFunction =
    core::Function<const analysis::AnalysisDomainMesh&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

/// @brief Factory signature types for serial scalar criteria
using SerialCriterionRegistrationTypes = std::tuple<CriterionFunction, CriterionInput>;
/// @brief Factory signature types for parallel scalar criteria
using ParallelCriterionRegistrationTypes = std::tuple<CriterionFunction, CriterionInput, boost::mpi::communicator>;
/// @brief Factory signature types for serial vector criteria
using SerialVectorCriterionRegistrationTypes = std::tuple<VectorCriterionFunction, CriterionInput>;
/// @brief Factory signature types for parallel vector criteria
using ParallelVectorCriterionRegistrationTypes =
    std::tuple<VectorCriterionFunction, CriterionInput, boost::mpi::communicator>;

/// @brief All factory argument lists that can be registered.
/// @note To add a new factory type, a new signature tuple should be created along with associated traits. The trait
/// index must match the index into this tuple.
/// @sa CriterionTraits
using FactoryRegistrationTypes = std::tuple<ParallelCriterionRegistrationTypes,
                                            SerialCriterionRegistrationTypes,
                                            ParallelVectorCriterionRegistrationTypes,
                                            SerialVectorCriterionRegistrationTypes>;

/// @brief Checks if a criterion function is registered with name @a aFunctionName and with any factory (any set of
/// traits).
[[nodiscard]] auto is_criterion_function_registered(const std::string_view aFunctionName) -> bool;

/// @brief Checks if a criterion function is registered with name @a aFunctionName and with traits @a aTraits.
[[nodiscard]] auto criterion_function_has_traits(const std::string_view aFunctionName, const CriterionTraits aTraits)
    -> bool;

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

namespace detail
{
template <typename FactoryArgTuple, typename U>
struct FactoryFromTypes
{
};

template <typename FactoryArgTuple, std::size_t... kIndices>
struct FactoryFromTypes<FactoryArgTuple, std::index_sequence<kIndices...>>
{
    using Factory = core::FactoryRegistration<std::tuple_element_t<kIndices, FactoryArgTuple>...>;
};

template <std::size_t kIndex>
using FactoryTypesAtIndex = std::tuple_element_t<kIndex, FactoryRegistrationTypes>;

template <std::size_t kIndex>
using FactoryRegistrationWithTraits =
    typename FactoryFromTypes<FactoryTypesAtIndex<kIndex>,
                              std::make_index_sequence<std::tuple_size_v<FactoryTypesAtIndex<kIndex>>>>::Factory;

}  // namespace detail

/// @brief Factory registration type template for registering criteria.
///
/// This template chooses different registration objects based on the template parameter traits.
/// @tparam kParallelization Chooses the parallel or serial criteria factory.
/// @tparam kFunctionDimension Chooses the scalar or vector criteria factory.
template <Parallelization kParallelization, FunctionDimension kFunctionDimension>
using CriterionRegistration =
    detail::FactoryRegistrationWithTraits<trait_index(CriterionTraits{kParallelization, kFunctionDimension})>;

static_assert(
    number_of_traits() == std::tuple_size_v<FactoryRegistrationTypes>,
    "The number of entries in FactoryRegistration types must match the number of combinations of criterion traits.");
}  // namespace plato::criteria::library

#endif
