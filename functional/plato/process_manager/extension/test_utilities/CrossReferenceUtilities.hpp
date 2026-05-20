#ifndef PLATO_PROCESS_MANAGER_EXTENSION_TEST_UTILITIES_CROSSREFERENCEUTILITIES
#define PLATO_PROCESS_MANAGER_EXTENSION_TEST_UTILITIES_CROSSREFERENCEUTILITIES

#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"

namespace plato::process_manager::extension::test_utilities
{
template <typename ComponentInput>
[[nodiscard]] auto cross_reference()
{
    return input_parser::CrossReference<input_parser::ComponentTypeOfInputBlock<ComponentInput>::mValue>{
        input_parser::block_name<ComponentInput>(), input_parser::InputBlockWrapper{ComponentInput{}}};
}
}  // namespace plato::process_manager::extension::test_utilities

#endif
