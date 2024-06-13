#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONREGISTRATION

#include <boost/mpi/communicator.hpp>
#include <string_view>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::core
{
struct MeshProxy;
}  // namespace plato::core

namespace plato::criteria::library
{
struct CriterionInput
{
    input_parser::FileName mSharedLibraryPath;
    unsigned int mNumberOfProcessors;
    input_parser::FileList mInputFiles;
};

using CriterionFunction = core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;
using CriterionRegistration = core::FactoryRegistration<CriterionFunction, CriterionInput>;
using ParallelCriterionRegistration =
    core::FactoryRegistration<CriterionFunction, CriterionInput, boost::mpi::communicator>;

[[nodiscard]] bool is_criterion_function_registered(const std::string_view aFunctionName);
[[nodiscard]] bool is_parallel_criterion_function_registered(const std::string_view aFunctionName);

}  // namespace plato::criteria::library

#endif
