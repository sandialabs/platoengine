#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION

#include <boost/mpi/communicator.hpp>
#include <boost/optional.hpp>
#include <set>
#include <string_view>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/utilities/EnumIndexing.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::criteria::library
{
struct CriterionInput
{
    unsigned int mNumberOfProcessors;
    input_parser::FileList mInputFiles;
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

/// @brief Checks if a criterion function is registered with name @a aFunctionName and with traits @a aTraits.
[[nodiscard]] auto is_criterion_function_registered(const std::string_view aFunctionName, const CriterionTraits aTraits)
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
using SerialCriterionRegistration = core::FactoryRegistration<CriterionFunction, CriterionInput>;
using ParallelCriterionRegistration =
    core::FactoryRegistration<CriterionFunction, CriterionInput, boost::mpi::communicator>;
using SerialVectorCriterionRegistration = core::FactoryRegistration<VectorCriterionFunction, CriterionInput>;
using ParallelVectorCriterionRegistration =
    core::FactoryRegistration<VectorCriterionFunction, CriterionInput, boost::mpi::communicator>;

using FactoryRegistrars = std::tuple<ParallelCriterionRegistration,
                                     SerialCriterionRegistration,
                                     ParallelVectorCriterionRegistration,
                                     SerialVectorCriterionRegistration>;

template <std::size_t Index>
using FactoryRegistrationWithTraits = std::tuple_element_t<Index, FactoryRegistrars>;

/// @brief Returns an index into FactoryRegistrars corresponding to the traits in @a CriterionTraits.
[[nodiscard]] constexpr auto factory_index(const CriterionTraits aCriterionTraits) -> std::size_t;

/// @brief Returns an index into FactoryRegistrars corresponding to the traits in @a CriterionTraits.
[[nodiscard]] constexpr auto factory_index(Parallelization aParallelization, FunctionDimension aDimension)
    -> std::size_t;
}  // namespace detail

/// @brief Factory registration type template for registering criteria.
///
/// This template chooses different registration objects based on the template parameter traits.
/// @tparam kParallelization Chooses the parallel or serial criteria factory.
/// @tparam kFunctionDimension Chooses the scalar or vector criteria factory.
template <Parallelization kParallelization, FunctionDimension kFunctionDimension>
using CriterionRegistration =
    detail::FactoryRegistrationWithTraits<detail::factory_index(CriterionTraits{kParallelization, kFunctionDimension})>;

namespace detail
{
constexpr auto factory_index(const CriterionTraits aCriterionTraits) -> std::size_t
{
    return utilities::enum_index(aCriterionTraits.mParallelization, aCriterionTraits.mDimension);
}

constexpr auto factory_index(const Parallelization aParallelization, const FunctionDimension aDimension) -> std::size_t
{
    return factory_index(CriterionTraits{aParallelization, aDimension});
}
}  // namespace detail

}  // namespace plato::criteria::library

#endif
