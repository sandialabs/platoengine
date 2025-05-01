include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(FOLDERS library 
            extension)

set(LINK_LIBS PlatoFunctionalFilter
              PlatoFunctionalSTKIO
              PlatoFunctionalKrino 
              PlatoFunctionalMesh )

create_plato_library(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_library(PlatoFunctionalGeometryTestUtilities "extension/test_utilities" "PlatoFunctionalKrinoTestUtilities;PlatoFunctionalGeometry")

  # Build unit tester
  set(FOLDERS library/unittest 
              extension/unittest)
  set(LINK_LIBS PlatoFunctionalKrinoTestUtilities
                PlatoFunctionalSTKIOTestUtilities
                PlatoFunctionalFilterTestUtilities
                PlatoFunctionalGeometryTestUtilities)
  create_plato_unittester(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

  set(NUM_RANKS_FOR_TEST 1)
  create_plato_parallel_unittester(PlatoFunctionalGeometry "extension/parallel_unittest" ${NUM_RANKS_FOR_TEST} "${LINK_LIBS}")

endif()
