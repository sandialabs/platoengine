#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTTARGET
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTTARGET

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace plato::criteria::library
{
/// @brief A container for constraint targets, this handles logic required for vector constraints with a single value
/// for the entire target vector, or a vector with different components.
class ConstraintTarget
{
   public:
    explicit ConstraintTarget(double aTarget);
    explicit ConstraintTarget(std::vector<double> aTargets);

    /// @brief Returns the size of the targets.
    [[nodiscard]] auto size() const -> std::size_t;

    /// @brief Returns the constraint targets as a vector. If this object was constructed with a scalar, the returned
    /// vector will be a vector with size @a aConstraintDimension filled with that value. If this object was constructed
    /// with a vector, the returned vector will match that used on construction.
    /// @pre If this object was constructed with a vector, @a aConstraintDimension must match that vector's size.
    [[nodiscard]] auto value(std::size_t aConstraintDimension) const -> std::vector<double>;

    [[nodiscard]] auto operator==(const ConstraintTarget&) const -> bool = default;

   private:
    std::vector<double> mTargets;
};

/// @brief Creates a ConstraintTarget by mapping the targets given in @a aConstraintTargets to the names in @a
/// aComponentNames, ensuring the targets are ordered correctly.
/// @param aComponentIndexAssociations Defines the ordering of the components. The targets used to construct
/// ConstraintTarget will be ordered in increasing order of ID.
[[nodiscard]] auto make_constraint_target(const std::vector<std::pair<std::string, double>>& aConstraintTargets,
                                          const std::map<std::size_t, std::string>& aComponentNameIndexAssociations)
    -> ConstraintTarget;

}  // namespace plato::criteria::library

#endif
