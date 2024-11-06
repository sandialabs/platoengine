#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_CONSTRAINTS
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_CONSTRAINTS

#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief Internal-to-snopt constraint data that assumes scalar constraints
struct ConstraintData
{
    CriterionType mFunction;
    double mTarget = 0.0;
    Linearity mLinearity = Linearity::kLinear;
};

using ConstraintVectorType = std::vector<ConstraintData>;

/// @brief A helper class for managing constraints.
///
/// SNOPT requires specific handling of linear vs. non-linear constraints, and so this class
/// helps manage both, but still use a single vector of all constraints.
class SNOPTConstraints
{
   public:
    /// @brief Construction from an unpartitioned vector of Constraint objects, which will be partitioned by linearity.
    /// @post The vector returned by release and constraint member functions will be partitioned such that all
    /// non-linear constraints are ordered before linear constraints.
    SNOPTConstraints(InterfaceConstraintVectorType&& aConstraints, const std::size_t aNumberOfDesignVariables);

    [[nodiscard]] auto numberOfLinearConstraints() const -> std::size_t;
    [[nodiscard]] auto numberOfNonlinearConstraints() const -> std::size_t;

    [[nodiscard]] auto linearConstraintsBegin() const -> ConstraintVectorType::const_iterator;
    [[nodiscard]] auto linearConstraintsEnd() const -> ConstraintVectorType::const_iterator;

    [[nodiscard]] auto nonlinearConstraintsBegin() const -> ConstraintVectorType::const_iterator;
    [[nodiscard]] auto nonlinearConstraintsEnd() const -> ConstraintVectorType::const_iterator;

    [[nodiscard]] auto constraints() const -> const ConstraintVectorType&;

    [[nodiscard]] auto release() && -> ConstraintVectorType;

   private:
    ConstraintVectorType mConstraints;
    ConstraintVectorType::const_iterator mLinearConstraintsBeginIterator;
};

/// @brief Generates the constraint bounds, in the order of the constraints in @a aConstraints.
[[nodiscard]] auto constraint_bounds(const SNOPTConstraints& aConstraints) -> SNOPTBounds;

/// @brief Generates the objective and constraint bounds, with the objective bounds first and the constraints ordered as
/// they appear in @a aConstraints.
[[nodiscard]] auto constraint_bounds_with_unbounded_objective(const SNOPTConstraints& aConstraints) -> SNOPTBounds;

/// @brief Remove affine term from the constraint bounds to conform to SNOPT's interface
/// Example: \f$c(x) = ax + b, l \leq c(x) \leq u \rightarrow l - c(0) \leq a*x \leq u - c(0)\f$
[[nodiscard]] auto constraints_with_affine_offset_removed(ConstraintVectorType&& aConstraints,
                                                          const std::size_t aNumberOfDesignVariables)
    -> ConstraintVectorType;

/// @brief Expands any vector constraints into individual scalar constraints, converting all Jacobian types to
/// gradients.
///
/// The purpose of this function is to adapt the plato interface that uses vector constraints for all constraint types
/// to scalar constraints, which is what snopt assumes.
[[nodiscard]] auto constraints_with_vectors_expanded(InterfaceConstraintVectorType&& aConstraints)
    -> ConstraintVectorType;

namespace detail
{
/// @brief Returns the total number of constraint components (dimension) associated with the vector constraints @a
/// aVectorConstraint.
[[nodiscard]] auto total_number_of_scalar_constraints(const InterfaceConstraintVectorType& aVectorConstraint)
    -> std::size_t;

/// @brief Expands a single vector constraint into a vector of scalar constraints.
[[nodiscard]] auto constraint_with_vectors_expanded(const InterfaceConstraintType& aConstraint) -> ConstraintVectorType;
}  // namespace detail

}  // namespace plato::third_party_integration::snopt

#endif
