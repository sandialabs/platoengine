#include "plato/integration_tests/test_mass_criteria/MassObjective.hpp"

#include <numeric>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"

namespace plato::integration_tests::test_mass_criteria
{

MassObjective::MassObjective(const double aDensity) : mDensity(aDensity) {}

double MassObjective::mass(const std::string_view aMeshFileName) const
{
    const auto tMesh = mesh::Mesh{aMeshFileName};
    const auto tVolumes = mesh::MeshQuantities{tMesh}.designDomainElementVolumes();
    return std::accumulate(tVolumes.begin(), tVolumes.end(), 0.0,
                           [tDensity = mDensity](const double aTotal, const double aVolume)
                           { return aTotal + tDensity * aVolume; });
}

}  // namespace plato::integration_tests::test_mass_criteria
