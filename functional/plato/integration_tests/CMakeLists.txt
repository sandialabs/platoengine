include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

create_plato_library(PlatoTestMassObjective test_mass_objective PlatoFunctionalCriteria)

set(LINK_LIBS PlatoFunctionalLinearAlgebra
              PlatoFunctionalCore
              PlatoFunctionalTestUtilities)
create_plato_library(PlatoFunctionalIntegrationTestUtilities utilities "${LINK_LIBS}")

# Build serial unit tester
set(LINK_LIBS TeuchosCore::teuchoscore 
              ROL::rol
              PlatoFunctional
              PlatoFunctionalIntegrationTestUtilities
              PlatoTestMassObjective)
create_plato_integration_tester(PlatoFunctional_IntegrationUnitTester serial "${LINK_LIBS}")

# Parallel tests
set(LINK_LIBS PlatoFunctional 
              PlatoFunctionalIntegrationTestUtilities)
set(NUM_RANKS_FOR_TEST 4)
create_plato_parallel_integration_tester(PlatoFunctional_IntegrationParallelUnitTester parallel "${LINK_LIBS}" ${NUM_RANKS_FOR_TEST})
