include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities STKSearch::stk_search PlatoFunctionalTPICommon)
create_plato_library(PlatoFunctionalSTKSearch "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalSTKSearch unittest)

  set(NUM_RANKS_FOR_TEST 4)
  set(LINK_LIBS PlatoFunctionalSTKSearch)
  create_plato_parallel_integration_tester(PlatoFunctionalSTKSearch_ParallelUnitTester parallel_unittest "${LINK_LIBS}" ${NUM_RANKS_FOR_TEST})

endif()
