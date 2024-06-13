#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"

#include <numeric>

#include "plato/utilities/STKVolumeUtilities.hpp"

namespace plato::integration_tests::test_mass_objective
{

MassObjective::MassObjective(const double aDensity) : mDensity(aDensity) {}

double MassObjective::mass(const std::string_view aMeshFileName) const
{
    std::shared_ptr<stk::mesh::BulkData> tBulk = utilities::read_mesh_bulk_data(aMeshFileName);

    const stk::mesh::EntityVector tElements = utilities::element_vector(*tBulk);

    return std::accumulate(tElements.begin(), tElements.end(), 0.0,
                           [&tBulk, this](const double aResult, const stk::mesh::Entity& aElement)
                           { return aResult + mDensity * utilities::element_volume(aElement, *tBulk); });
}

}  // namespace plato::integration_tests::test_mass_objective
