#include "plato/criteria/library/CriterionRegistration.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/input_parser/InputBlocks.hpp"

namespace plato::criteria::library
{
bool is_criterion_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<CriterionFunction, CriterionInput>(aFunctionName);
}

bool is_parallel_criterion_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<CriterionFunction, CriterionInput, boost::mpi::communicator>(
        aFunctionName);
}

}  // namespace plato::criteria::library
