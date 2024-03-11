#include "plato/filter/library/FilterFactory.hpp"

#include <type_traits>

#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::library
{
FilterFunction make_filter_function(const input_parser::density_topology& aInput)
{
    const std::string tFilterName = input_parser::kFilterTypesTable.toString(aInput.filter_type.value()).value();
    const std::optional<FilterFunction> tFilter =
        core::create_object_from_factory<FilterFunction, FilterInput>(tFilterName, aInput);

    if (tFilter)
    {
        return std::move(tFilter).value();
    }
    else
    {
        throw plato::utilities::Exception{"Unknown filter_type"};
    }
}
}  // namespace plato::filter::library
