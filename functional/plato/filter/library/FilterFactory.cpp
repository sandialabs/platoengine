#include "plato/filter/library/FilterFactory.hpp"

#include <type_traits>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::library
{
FilterFunction make_filter_function(const ValidatedFilterInput& aInput)
{
    std::optional<FilterFunction> tFilter = core::create_object_from_factory<FilterFunction, ValidatedFilterInput>(
        core::block_name(aInput.rawInput()), aInput);

    if (tFilter)
    {
        return std::move(tFilter).value();
    }
    else
    {
        throw plato::utilities::Exception{"Unknown filter_type. Requested name: " +
                                          core::block_name(aInput.rawInput())};
    }
}

auto make_filter_function(const std::shared_ptr<FilterInterface>& aFilter) -> FilterFunction
{
    return filter::library::FilterFunction{
        [aFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return aFilter->filter(aAnalysisDomainMesh); },
        [aFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return filter::library::make_filter_jacobian(aFilter, aAnalysisDomainMesh); },
        [aFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return filter::library::make_filter_adjoint_jacobian(aFilter, aAnalysisDomainMesh); }};
}

}  // namespace plato::filter::library
