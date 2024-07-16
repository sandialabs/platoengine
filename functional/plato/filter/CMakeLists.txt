include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(FOLDERS library
            extension)

set(LINK_LIBS PlatoFunctionalCore
              PlatoFunctionalLinearAlgebra
              PlatoFunctionalServices
              PlatoFunctionalSTKIO
              PlatoFunctionalSTKSearch
              PlatoFunctionalTpetra)

create_plato_library(PlatoFunctionalFilter "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build test filter shared lib
  create_plato_library(PlatoIdentityFilter test_utilities PlatoFunctionalFilter)

  # Build unit tester
  set(FOLDERS library/unittest
              extension/unittest)
  create_plato_unittester(PlatoFunctionalFilter "${FOLDERS}")

  set(DIRECTORIES extension/parallel_unittest)
  set(NUM_RANKS_FOR_TEST 4)
  create_plato_parallel_unittester(PlatoFunctionalFilter "${DIRECTORIES}" ${NUM_RANKS_FOR_TEST})

endif()
