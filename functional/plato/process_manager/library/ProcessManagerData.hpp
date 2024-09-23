#ifndef PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERDATA
#define PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERDATA

#include <Teuchos_ParameterList.hpp>
#include <vector>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"

namespace plato::process_manager::library
{
///@brief Sef of functions used in a ProcessManager to evaluate geometries, objectives, and constraints.
struct ProcessManagerData
{
    plato::geometry::library::FactoryTypes mGeometry;
    plato::criteria::library::ObjectiveFunction mObjective;
    std::vector<plato::criteria::library::Constraint<const analysis::AnalysisDomainMesh&>> mConstraints;
};

///@brief Convert validated parsed input into a populated ProcessManagerData struct
[[nodiscard]] ProcessManagerData make_process_manager_data(const ValidatedInput& aData);

}  // namespace plato::process_manager::library

#endif
