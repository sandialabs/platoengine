#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTTARGET
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTTARGET

#include <vector>

namespace plato::criteria::library
{
/// @brief A container for constraint targets, this handles logic required for vector constraints with a single value
/// for the entire target vector, or a vector with different components.
class ConstraintTarget
{
   public:
    ConstraintTarget(double aTarget);
    ConstraintTarget(std::vector<double> aTargets);

    /// @brief Returns the size of the targets.
    [[nodiscard]] auto size() const -> std::size_t;

    /// @brief Returns the constraint targets as a vector. If this object was constructed with a scalar, the returned
    /// vector will be a vector with size @a aConstraintDimension filled with that value. If this object was constructed
    /// with a vector, the returned vector will match that used on construction.
    /// @pre If this object was constructed with a vector, @a aConstraintDimension must match that vector's size.
    [[nodiscard]] auto value(std::size_t aConstraintDimension) const -> std::vector<double>;

   private:
    std::vector<double> mTargets;
};
}  // namespace plato::criteria::library

#endif
