#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_CONSTRAINTS
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_CONSTRAINTS

#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief A helper class for managing constraints.
///
/// SNOPT requires specific handling of linear vs. non-linear constraints, and so this class
/// helps manage both, but still use a single vector of all constraints.
class SNOPTConstraints
{
   public:
    /// @brief Construction from an unpartitioned vector of Constraint objects, which will be partitioned by linearity.
    /// @post The vector returnd by release and constraint member functions will be partitioned such that all
    /// non-linear constraints are ordered before linear constraints.
    SNOPTConstraints(ConstraintVectorType&& aConstraints);

    [[nodiscard]] auto numberOfLinearConstraints() const -> std::size_t;
    [[nodiscard]] auto numberOfNonlinearConstraints() const -> std::size_t;

    [[nodiscard]] auto linearConstraintsBegin() const -> ConstraintVectorType::const_iterator;
    [[nodiscard]] auto linearConstraintsEnd() const -> ConstraintVectorType::const_iterator;

    [[nodiscard]] auto nonlinearConstraintsBegin() const -> ConstraintVectorType::const_iterator;
    [[nodiscard]] auto nonlinearConstraintsEnd() const -> ConstraintVectorType::const_iterator;

    [[nodiscard]] auto constraints() const -> const ConstraintVectorType&;

    [[nodiscard]] auto release() && -> ConstraintVectorType;

   private:
    /// @brief Remove affine term from the constraint bounds to conform to SNOPT's interface
    /// Example: c(x) = a*x + b, l <= c(x) <= u => l - c(0) <= a*x <= u - c(0)
    void subtract_affine_offset_from_bounds();

    ConstraintVectorType mConstraints;
    ConstraintVectorType::iterator mLinearConstraintsBeginIterator;
};

/// @brief Generates the constraint bounds, in the order of the constraints in @a aConstraints.
auto constraint_bounds(const SNOPTConstraints& aConstraints) -> SNOPTBounds;

/// @brief Generates the objective and constraint bounds, with the objective bounds first and the constraints ordered as
/// they appear in @a aConstraints.
auto constraint_bounds_with_unbounded_objective(const SNOPTConstraints& aConstraints) -> SNOPTBounds;

}  // namespace plato::third_party_integration::snopt

#endif
