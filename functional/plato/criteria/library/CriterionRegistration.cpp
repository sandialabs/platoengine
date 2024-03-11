#include "plato/criteria/library/CriterionRegistration.hpp"

#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
bool is_criterion_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<CriterionFunction, CriterionInput>(aFunctionName);
}
}  // namespace plato::criteria::library
