#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONINTERFACE
#define PLATO_CRITERIA_LIBRARY_CRITERIONINTERFACE

#include <memory>
#include <string>
#include <vector>

#include "plato/core/MeshProxy.hpp"

namespace plato::criteria::library
{
/// @brief Interface for implementing criteria loaded from shared libraries.
///
/// Any external criterion (objective or constraint) must inherit this interface.
/// External shared libraries must then provide an exported function that constructs and
/// returns a `std::unique_ptr` to this interface class. The exported function must be
/// associated with a criterion name in a config file. The following is an example of the
/// required declaration for serial criteria:
/// @code{.cpp}
/// extern "C" auto plato_create_criterion(const std::vector<std::string>& aFileNames)
///  -> std::unique_ptr<::plato::criteria::library::CriterionInterface>;
/// @endcode
/// and for parallel criteria:
/// @code{.cpp}
/// extern "C" auto plato_create_parallel_criterion(const std::vector<std::string>& aFileNames, MPI_Comm aComm)
///  -> std::unique_ptr<::plato::criteria::library::CriterionInterface>;
/// @endcode
/// `extern "C"` ensures that the function name is not mangled in the shared library symbols.
class CriterionInterface
{
   public:
    CriterionInterface() = default;
    virtual ~CriterionInterface() = default;

    /// @note When implementing a constraint, the target value will be subtracted in the optimizer interface.
    /// A criterion that is a constraint should then just evaluate without considering any target value.
    virtual double value(const core::MeshProxy& aMeshProxy) const = 0;
    virtual std::vector<double> gradient(const core::MeshProxy& aMeshProxy) const = 0;

    CriterionInterface(const CriterionInterface&) = delete;
    CriterionInterface& operator=(const CriterionInterface&) = delete;
    CriterionInterface(CriterionInterface&&) = delete;
    CriterionInterface& operator=(CriterionInterface&&) = delete;
};

}  // namespace plato::criteria::library

#endif
