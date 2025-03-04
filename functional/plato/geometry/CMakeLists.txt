include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(FOLDERS library 
            extension)

set(LINK_LIBS PlatoFunctionalFilter
              PlatoFunctionalSTKIO
              PlatoFunctionalKrinoLib 
              PlatoFunctionalMesh )

create_plato_library(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build unit tester
  set(FOLDERS library/unittest 
              extension/unittest)
  set(LINK_LIBS PlatoFunctionalSTKIOTestUtilities
                PlatoFunctionalFilterTestUtilities)
  create_plato_unittester(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

  set(NUM_RANKS_FOR_TEST 2)
  create_plato_parallel_unittester(PlatoFunctionalGeometry "extension/parallel_unittest" ${NUM_RANKS_FOR_TEST} "${LINK_LIBS}")

endif()
