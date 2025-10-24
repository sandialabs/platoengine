#include "plato/criteria/library/ConstraintTarget.hpp"

#include <cassert>

namespace plato::criteria::library
{
ConstraintTarget::ConstraintTarget(const double aTarget) : mTargets(1U, aTarget) {}
ConstraintTarget::ConstraintTarget(std::vector<double> aTargets) : mTargets(std::move(aTargets)) {}

auto ConstraintTarget::size() const -> std::size_t { return mTargets.size(); }

auto ConstraintTarget::value(const std::size_t aConstraintDimension) const -> std::vector<double>
{
    if (mTargets.size() == 1U)
    {
        return std::vector<double>(aConstraintDimension, mTargets.front());
    }

    assert(aConstraintDimension == mTargets.size());
    return mTargets;
}
}  // namespace plato::criteria::library
