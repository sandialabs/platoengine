include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Krino::krino_lib PlatoFunctionalUtilities)
create_plato_library(PlatoFunctionalKrino "." "${LINK_LIBS}")

if( UNIT_TESTING )

  set(LINK_LIBS PlatoFunctionalKrino 
                PlatoFunctionalTestUtilities)
  create_plato_library(PlatoFunctionalKrinoTestUtilities "./test_utilities" "${LINK_LIBS}")
  
  
  set(LINK_LIBS PlatoFunctionalSTKIOTestUtilities 
                PlatoFunctionalKrinoTestUtilities)
  create_plato_unittester(PlatoFunctionalKrino "unittest" "${LINK_LIBS}")

  set(NUM_RANKS_FOR_TEST 4)
  set(LINK_LIBS PlatoFunctionalKrino
                PlatoFunctionalSTKIOTestUtilities 
                PlatoFunctionalKrinoTestUtilities)
  create_plato_parallel_integration_tester(PlatoFunctionalKrino_ParallelUnitTester parallel_unittest "${LINK_LIBS}" ${NUM_RANKS_FOR_TEST})

endif()
