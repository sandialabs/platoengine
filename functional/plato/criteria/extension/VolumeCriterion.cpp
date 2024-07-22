#include "plato/criteria/extension/VolumeCriterion.hpp"

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshProxyViews.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::criteria::extension
{

namespace
{
[[maybe_unused]] static auto kVolumeConstraintRegistration =
    library::CriterionRegistration{library::builtin_criterion_registration_name(VolumeCriterion::kVolumeCriterionName),
                                   [](const library::CriterionInput&) { return make_volume_constraint_function(); }};

[[maybe_unused]] static auto kVolumeFractionConstraintRegistration = library::CriterionRegistration{
    library::builtin_criterion_registration_name(VolumeCriterion::kVolumeFractionCriterionName),
    [](const library::CriterionInput&) { return make_volume_fraction_constraint_function(); }};

}  // namespace

double VolumeCriterion::f(const mesh::MeshProxy& aMeshProxy) const
{
    const auto tMesh = mesh::MeshQuantities{mesh::Mesh{aMeshProxy.mFileName}};
    auto tScaledVolumes = tMesh.elementVolumes();
    const auto tMeshView = mesh::MeshProxyDensitiesView{aMeshProxy};
    std::transform(tScaledVolumes.cbegin(), tScaledVolumes.cend(), tMeshView.begin(), tScaledVolumes.begin(),
                   [](const double aVolume, const mesh::Density aDensity) { return aDensity.mDensity * aVolume; });
    return mScaleFactor * utilities::pair_wise_accumulate(tScaledVolumes);
}

linear_algebra::DynamicVector<double> VolumeCriterion::df(const mesh::MeshProxy& aMeshProxy) const
{
    const auto tMesh = mesh::MeshQuantities{mesh::Mesh{aMeshProxy.mFileName}};
    auto tJacobian = linear_algebra::DynamicVector<double>{tMesh.elementVolumes()};
    tJacobian = mScaleFactor * std::move(tJacobian);
    return tJacobian;
}

auto make_volume_constraint_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const mesh::MeshProxy&>
{
    return core::make_function([](const mesh::MeshProxy& mesh) { return VolumeCriterion{}.f(mesh); },
                               [](const mesh::MeshProxy& mesh) { return VolumeCriterion{}.df(mesh); });
}

auto make_volume_fraction_constraint_function()
    -> core::Function<double, linear_algebra::DynamicVector<double>, const mesh::MeshProxy&>
{
    return core::make_function(
        [](const mesh::MeshProxy& mesh)
        {
            const double tVolumeTotal = third_party_integration::stk_io::mesh_volume(
                *third_party_integration::stk_io::read_mesh_bulk_data(mesh.mFileName));
            return VolumeCriterion{1.0 / tVolumeTotal}.f(mesh);
        },
        [](const mesh::MeshProxy& mesh)
        {
            const double tVolumeTotal = third_party_integration::stk_io::mesh_volume(
                *third_party_integration::stk_io::read_mesh_bulk_data(mesh.mFileName));
            return VolumeCriterion{1.0 / tVolumeTotal}.df(mesh);
        });
}

}  // namespace plato::criteria::extension
