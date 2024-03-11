#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONINTERFACE
#define PLATO_CRITERIA_LIBRARY_CRITERIONINTERFACE

#include <memory>
#include <string>
#include <vector>

#include "plato/core/MeshProxy.hpp"

namespace plato::criteria::library
{
static constexpr std::string_view kCreateCriterionFunctionName = "plato_create_criterion";

/// @brief Interface for implementing criteria loaded from shared libraries.
///
/// Any external criterion (objective or constraint) should inherit this interface
/// and implement plato_create_criterion.
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

namespace plato
{
extern "C" std::unique_ptr<criteria::library::CriterionInterface> plato_create_criterion(
    const std::vector<std::string>& aFileNames);
}

#endif
