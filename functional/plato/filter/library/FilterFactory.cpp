#include "plato/filter/library/FilterFactory.hpp"

#include <type_traits>

#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::library
{
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
