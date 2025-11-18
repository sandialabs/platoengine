#include "plato/criteria/library/ConstraintInputBlock.hpp"

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

DEFINE_ENUM_SYMBOL_TABLE(ConstraintTypes, plato::input_parser)

namespace plato::criteria::library
{
namespace
{
[[maybe_unused]] static auto kConstraintParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::constraint>{};
}

auto to_vector(const ConstraintValueList& aConstraintList) -> ConstraintComponentVector
{
    auto tConstraintValues = utilities::reserved_container<ConstraintComponentVector>(aConstraintList.mList.size());
    std::ranges::transform(
        aConstraintList.mList, std::back_inserter(tConstraintValues), [](const auto& aComponentNameAndValue)
        { return std::make_pair(aComponentNameAndValue.component.mToken, aComponentNameAndValue.target); });
    return tConstraintValues;
}

}  // namespace plato::criteria::library
