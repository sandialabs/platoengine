#ifndef PLATO_CRITERIA_LIBRARY_VECTORCRITERIONINTERFACE
#define PLATO_CRITERIA_LIBRARY_VECTORCRITERIONINTERFACE

#include <memory>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::criteria::library
{
/// @brief Interface for implementing vector criteria loaded from shared libraries.
///
/// Any external criterion (objective or constraint) must inherit this interface or the scalar version.
/// External shared libraries must then provide an exported function that constructs and
/// returns a `std::unique_ptr` to this interface class. The exported function must be
/// associated with a criterion name in a config file. The following is an example of the
/// required declaration for serial criteria:
/// @code{.cpp}
/// extern "C" auto plato_create_vector_criterion(const std::vector<std::string>& aFileNames)
///  -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>;
/// @endcode
/// and for parallel criteria:
/// @code{.cpp}
/// extern "C" auto plato_create_parallel_vector_criterion(const std::vector<std::string>& aFileNames, MPI_Comm aComm)
///  -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>;
/// @endcode
/// `extern "C"` ensures that the function name is not mangled in the shared library symbols.
class VectorCriterionInterface
{
   public:
    VectorCriterionInterface() = default;
    virtual ~VectorCriterionInterface() = default;

    /// @note When implementing a constraint, the target value will be subtracted in the optimizer interface.
    /// A criterion that is a constraint should then just evaluate without considering any target value.
    /// Vector constraints share the same target value.
    virtual std::vector<double> value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const = 0;
    virtual std::vector<double> jacobianTimesVector(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                    const std::vector<double>& aDirectionVector) const = 0;
    virtual std::vector<double> adjointJacobianTimesVector(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                           const std::vector<double>& aDualVector) const = 0;

    VectorCriterionInterface(const VectorCriterionInterface&) = delete;
    VectorCriterionInterface& operator=(const VectorCriterionInterface&) = delete;
    VectorCriterionInterface(VectorCriterionInterface&&) = delete;
    VectorCriterionInterface& operator=(VectorCriterionInterface&&) = delete;
};

}  // namespace plato::criteria::library

#endif
