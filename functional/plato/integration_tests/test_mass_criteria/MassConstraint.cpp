
#include "plato/integration_tests/test_mass_criteria/MassConstraint.hpp"

#include <iterator>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"

namespace plato::integration_tests::test_mass_criteria
{

MassConstraint::MassConstraint(const double aDensity) : mDensity(aDensity) {}

std::vector<double> MassConstraint::masses(std::string_view aMeshFileName) const
{
    const auto tMesh = mesh::Mesh{aMeshFileName};
    const auto tVolumes = mesh::MeshQuantities{tMesh}.designDomainElementVolumes();
    std::vector<double> tMasses;
    std::transform(tVolumes.begin(), tVolumes.end(), std::back_inserter(tMasses),
                   [this](const auto& aVolume) { return aVolume * mDensity; });

    return tMasses;
}

}  // namespace plato::integration_tests::test_mass_criteria
