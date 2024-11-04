#include "plato/process_manager/library/CrossLinkedInput.hpp"

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/CrossReferenceUtilities.hpp"

namespace plato::process_manager::library
{
CrossLinkedInput::CrossLinkedInput(input_parser::ParsedInput aInput, const CrossLinkKey&) : mInput{std::move(aInput)}
{
    apply_to_cross_references(mInput, [](auto& aField, const auto& aInputBlock, const auto& aFullInput)
                              { CrossLinkedInput::fillCrossReference(aField, aInputBlock, aFullInput); });
}

CrossLinkedInput make_cross_linked_input(input_parser::ParsedInput aInput)
{
    return CrossLinkedInput{std::move(aInput), CrossLinkKey{}};
}
}  // namespace plato::process_manager::library
