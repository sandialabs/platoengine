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

auto make_filter_function(const NewValidatedFilterInput& aInput) -> FilterFunction
{
    auto tFilter =
        core::create_object_from_factory<FilterFunction, NewValidatedFilterInput>(aInput.rawInput().mBlockName, aInput);

    if (tFilter)
    {
        return std::move(tFilter).value();
    }
    else
    {
        throw plato::utilities::Exception{"Unknown filter_type. Requested name: " + aInput.rawInput().mBlockName};
    }
}

}  // namespace plato::filter::library
