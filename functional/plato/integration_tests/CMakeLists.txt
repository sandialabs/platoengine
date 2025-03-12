include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalCriteria
              PlatoFunctionalTestUtilities
              PlatoFunctionalMesh)
create_plato_library(PlatoTestMassObjective test_mass_objective "${LINK_LIBS}")

set(LINK_LIBS PlatoFunctionalCriteria
              PlatoFunctionalTestUtilities
              PlatoFunctionalMesh)
create_plato_library(PlatoTestVectorConstraint test_vector_constraint "${LINK_LIBS}")

set(LINK_LIBS PlatoFunctionalLinearAlgebra
              PlatoFunctionalCore
              PlatoFunctionalTestUtilities)
create_plato_library(PlatoFunctionalIntegrationTestUtilities utilities "${LINK_LIBS}")

# Build serial unit tester
set(LINK_LIBS PlatoFunctionalTpetra
              PlatoFunctionalROL
              PlatoFunctionalSTKIO
              PlatoFunctionalSTKIOTestUtilities
              PlatoFunctional
              PlatoFunctionalIntegrationTestUtilities)
set(USE_FOLDERS unittest)
create_plato_integration_tester(PlatoFunctional_IntegrationUnitTester "${USE_FOLDERS}" "${LINK_LIBS}")

set(LINK_LIBS PlatoFunctionalSTKIO
              PlatoFunctionalSTKIOTestUtilities
              PlatoFunctionalProcessManager
              PlatoTestMassObjective
              PlatoTestVectorConstraint )
set(USE_FOLDERS test_mass_objective/unittest test_vector_constraint/unittest)
create_plato_integration_tester(PlatoFunctional_IntegrationMassLibUnitTester "${USE_FOLDERS}" "${LINK_LIBS}")

# Parallel tests
set(LINK_LIBS PlatoFunctional 
              PlatoFunctionalIntegrationTestUtilities)
set(NUM_RANKS_FOR_TEST 4)
create_plato_parallel_integration_tester(PlatoFunctional_IntegrationParallelUnitTester parallel_unittest "${LINK_LIBS}" ${NUM_RANKS_FOR_TEST})
