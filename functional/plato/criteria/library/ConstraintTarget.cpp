#include "plato/criteria/library/ConstraintTarget.hpp"

#include <cassert>

#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/TransformIf.hpp"

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

auto make_constraint_target(const std::vector<std::pair<std::string, double>>& aConstraintTargets,
                            const std::map<std::size_t, std::string>& aComponentIndexNameAssociations)
    -> ConstraintTarget
{
    const auto tFindComponent = [&aConstraintTargets](const std::string& aComponentName)
    {
        return std::ranges::find_if(aConstraintTargets, [&aComponentName](const auto& aTargetValueAndName)
                                    { return aTargetValueAndName.first == aComponentName; });
    };

    auto tTargets = utilities::reserved_container<std::vector<double>>(aConstraintTargets.size());
    utilities::transform_if(
        aComponentIndexNameAssociations, std::back_inserter(tTargets),
        [&tFindComponent](const auto& aComponentIndexAndName)
        { return tFindComponent(aComponentIndexAndName.second)->second; },
        [&aConstraintTargets, &tFindComponent](const auto& aComponentIndexAndName)
        { return tFindComponent(aComponentIndexAndName.second) != aConstraintTargets.end(); });
    return ConstraintTarget{std::move(tTargets)};
}

}  // namespace plato::criteria::library
