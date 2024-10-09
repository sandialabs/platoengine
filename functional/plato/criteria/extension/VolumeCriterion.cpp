#include "plato/criteria/extension/VolumeCriterion.hpp"

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
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

double fixed_domain_volume(const mesh::MeshQuantities& aMesh)
{
    const auto tElementVolumes = aMesh.fixedDomainElementVolumes();
    return utilities::pair_wise_accumulate(tElementVolumes);
}

}  // namespace

double VolumeCriterion::f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::MeshQuantities{mesh::Mesh{aAnalysisDomainMesh}};
    auto tScaledVolumes = tMesh.designDomainElementVolumes();
    const auto tMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    std::transform(tScaledVolumes.cbegin(), tScaledVolumes.cend(), tMeshView.begin(), tScaledVolumes.begin(),
                   [](const double aVolume, const analysis::ScalarFieldValue aDensity)
                   { return aDensity.mValue * aVolume; });

    return mScaleFactor * (utilities::pair_wise_accumulate(tScaledVolumes) + fixed_domain_volume(tMesh));
}

linear_algebra::DynamicVector<double> VolumeCriterion::df(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::MeshQuantities{mesh::Mesh{aAnalysisDomainMesh}};
    auto tJacobian = linear_algebra::DynamicVector<double>{tMesh.designDomainElementVolumes()};
    return mScaleFactor * std::move(tJacobian);
}

auto make_volume_constraint_function() -> library::CriterionFunction
{
    return core::make_function([](const analysis::AnalysisDomainMesh& mesh) { return VolumeCriterion{}.f(mesh); },
                               [](const analysis::AnalysisDomainMesh& mesh) { return VolumeCriterion{}.df(mesh); });
}

auto make_volume_fraction_constraint_function() -> library::CriterionFunction
{
    return core::make_function(
        [](const analysis::AnalysisDomainMesh& mesh)
        {
            const double tVolumeTotal = third_party_integration::stk_io::mesh_volume(
                *third_party_integration::stk_io::read_mesh_bulk_data(mesh.mFileName));
            return VolumeCriterion{1.0 / tVolumeTotal}.f(mesh);
        },
        [](const analysis::AnalysisDomainMesh& mesh)
        {
            const double tVolumeTotal = third_party_integration::stk_io::mesh_volume(
                *third_party_integration::stk_io::read_mesh_bulk_data(mesh.mFileName));
            return VolumeCriterion{1.0 / tVolumeTotal}.df(mesh);
        });
}

}  // namespace plato::criteria::extension
