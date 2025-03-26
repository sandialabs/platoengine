include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalInputParser 
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalCore "." "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build serial unit tester
  set(FOLDERS test_utilities
              unittest)
  create_plato_unittester(PlatoFunctionalCore "${FOLDERS}")

  set(FOLDERS test_utilities
              parallel_unittest)
  set(NUM_RANKS_FOR_TEST 2)
  create_plato_parallel_unittester(PlatoFunctionalCore "${FOLDERS}" ${NUM_RANKS_FOR_TEST})

endif()