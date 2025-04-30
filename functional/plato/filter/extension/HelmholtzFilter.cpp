#include "plato/filter/extension/HelmholtzFilter.hpp"

#include <filesystem>
#include <memory>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/filter/library/HashGeneration.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/BoostOptionalToStdOptional.hpp"

namespace plato::filter::extension
{
namespace
{
const auto kHelmholtzFilterLibName = std::filesystem::path{"libAnalyzeFunctionalInterface.so"};

library::FilterParameters to_filter_parameters(const input_parser::helmholtz_filter& aInput,
                                               const std::filesystem::path& aMeshFileName)
{
    return library::FilterParameters{
        /*.mFilterRadius=*/detail::get_filter_radius(aInput, aMeshFileName),
        /*.mBoundaryStickingPenalty=*/utilities::to_std_optional(aInput.boundary_sticking_penalty)};
}

// Static registration of input parser
[[maybe_unused]] static auto kHelmholtzFilterParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::helmholtz_filter>{};

[[maybe_unused]] static auto kHelmholtzFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::helmholtz_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::helmholtz_filter>(aInput);
        return library::make_filter_function_from_cache([&tInput]() { return detail::create_filter_cache(tInput); });
    }};

[[maybe_unused]] static auto kHelmholtzFilterValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::helmholtz_filter& aInput) { return detail::validate_filter_radius_bounds(aInput); },
        [](const input_parser::helmholtz_filter& aInput)
        { return validate_helmholtz_filter_boundary_sticking_penalty(aInput); }};

[[maybe_unused]] static auto kHelmholtzFilterMeshBasedValidationRegistration =
    input_validation::InputBlockValidationRegistration<std::filesystem::path>{
        [](const input_parser::helmholtz_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_filter_radius_with_mesh(aInput, aMeshPath); }};
}  // namespace

auto validate_helmholtz_filter_boundary_sticking_penalty(const input_parser::helmholtz_filter& aInput)
    -> std::optional<std::string>
{
    namespace pfu = plato::utilities;
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::helmholtz_filter>(), aInput.boundary_sticking_penalty,
        "boundary_sticking_penalty", pfu::unit_bounded());
}

namespace detail
{
library::FilterCache create_filter_cache(const input_parser::helmholtz_filter& aInput)
{
    return library::FilterCache{
        [aInput](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        {
            return std::shared_ptr<library::FilterInterface>(library::load_filter(
                to_filter_parameters(aInput, aAnalysisDomainMesh.mFileName), kHelmholtzFilterLibName));
        },
        [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return library::hash_mesh_coordinates(aAnalysisDomainMesh); }};
}
}  // namespace detail

}  // namespace plato::filter::extension
