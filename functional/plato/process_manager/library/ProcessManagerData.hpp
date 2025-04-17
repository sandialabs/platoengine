#ifndef PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERDATA
#define PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERDATA

#include <vector>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/input_validation/ValidatedInput.hpp"

namespace plato::process_manager::library
{
///@brief Sef of functions used in a ProcessManager to evaluate geometries, objectives, and constraints.
struct ProcessManagerData
{
    plato::geometry::library::FactoryTypes mGeometry;
    plato::criteria::library::ObjectiveFunction mObjective;
    std::vector<plato::criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>> mConstraints;
};

///@brief Convert validated parsed input into a populated ProcessManagerData struct
[[nodiscard]] auto make_process_manager_data(const input_validation::ValidatedInput& aData) -> ProcessManagerData;

}  // namespace plato::process_manager::library

#endif
