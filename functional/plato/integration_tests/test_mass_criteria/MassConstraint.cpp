
#include "plato/integration_tests/test_mass_criteria/MassConstraint.hpp"

#include <algorithm>
#include <iterator>
#include <string_view>
#include <vector>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"

namespace plato::integration_tests::test_mass_criteria
{

MassConstraint::MassConstraint(const double aDensity) : mDensity(aDensity) {}

auto MassConstraint::masses(std::string_view aMeshFileName) const -> std::vector<double>
{
    const auto tMesh = mesh::Mesh{aMeshFileName};
    const auto tVolumes = mesh::MeshQuantities{tMesh}.designDomainElementVolumes();
    std::vector<double> tMasses;
    std::transform(tVolumes.begin(), tVolumes.end(), std::back_inserter(tMasses),
                   [this](const auto& aVolume) { return aVolume * mDensity; });

    return tMasses;
}

}  // namespace plato::integration_tests::test_mass_criteria
