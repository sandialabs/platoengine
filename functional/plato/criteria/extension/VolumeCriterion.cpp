#include "plato/criteria/extension/VolumeCriterion.hpp"

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"
#include "plato/utilities/STKVolumeUtilities.hpp"

namespace plato::criteria::extension
{

namespace
{
[[maybe_unused]] static auto kVolumeConstraintRegistration =
    library::CriterionRegistration{input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kVolume).value(),
                                   [](const library::CriterionInput&) { return make_volume_constraint_function(); }};

[[maybe_unused]] static auto kVolumeFractionConstraintRegistration = library::CriterionRegistration{
    input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kVolumeFraction).value(),
    [](const library::CriterionInput&) { return make_volume_fraction_constraint_function(); }};

auto read_bulk_and_elements(const std::filesystem::path& aMeshFileName)
    -> std::pair<std::shared_ptr<stk::mesh::BulkData>, stk::mesh::EntityVector>
{
    std::shared_ptr<stk::mesh::BulkData> tBulk = utilities::read_mesh_bulk_data(aMeshFileName);
    assert(tBulk);
    const stk::mesh::EntityVector tElements = utilities::element_vector(*tBulk);
    return {tBulk, tElements};
}

}  // namespace

double VolumeCriterion::f(const core::MeshProxy& aMeshProxy) const
{
    const auto [tBulk, tElements] = read_bulk_and_elements(aMeshProxy.mFileName);
    assert(tElements.size() == aMeshProxy.mNodalDensities.size());
    std::vector<double> tScaledVolume;
    std::transform(aMeshProxy.mNodalDensities.begin(), aMeshProxy.mNodalDensities.end(), tElements.begin(),
                   std::back_inserter(tScaledVolume),
                   [&tBulkRef = *tBulk](const auto& aControl, const auto& aElement)
                   { return aControl * utilities::element_volume(aElement, tBulkRef); });

    return mScaleFactor * utilities::pair_wise_accumulate(tScaledVolume);
}

linear_algebra::DynamicVector<double> VolumeCriterion::df(const core::MeshProxy& aMeshProxy) const
{
    const auto [tBulk, tElements] = read_bulk_and_elements(aMeshProxy.mFileName);

    std::vector<double> tScaledVolume;
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tScaledVolume),
                   [this, &tBulkRef = *tBulk](const auto& aElement)
                   { return mScaleFactor * utilities::element_volume(aElement, tBulkRef); });

    return linear_algebra::DynamicVector<double>(std::move(tScaledVolume));
}

auto make_volume_constraint_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>
{
    return core::make_function([](const core::MeshProxy& mesh) { return VolumeCriterion{}.f(mesh); },
                               [](const core::MeshProxy& mesh) { return VolumeCriterion{}.df(mesh); });
}

auto make_volume_fraction_constraint_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>
{
    return core::make_function(
        [](const core::MeshProxy& mesh)
        {
            const double tVolumeTotal = utilities::mesh_volume(mesh.mFileName);
            return VolumeCriterion{1.0 / tVolumeTotal}.f(mesh);
        },
        [](const core::MeshProxy& mesh)
        {
            const double tVolumeTotal = utilities::mesh_volume(mesh.mFileName);
            return VolumeCriterion{1.0 / tVolumeTotal}.df(mesh);
        });
}

}  // namespace plato::criteria::extension
