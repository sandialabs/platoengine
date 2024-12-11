include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(FOLDERS library
            extension)

set(LINK_LIBS PlatoFunctionalCore
              PlatoFunctionalLinearAlgebra
              PlatoFunctionalMesh
              PlatoFunctionalServices
              PlatoFunctionalSTKIO
              PlatoFunctionalSTKSearch
              PlatoFunctionalTpetra
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalFilter "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build test utilities lib
  create_plato_library(PlatoFunctionalFilterTestUtilities test_utilities PlatoFunctionalFilter)

  # Build test filter shared lib
  create_plato_library(PlatoIdentityFilter test_identity_filter PlatoFunctionalFilter)

  # Build unit tester
  set(FOLDERS library/unittest
              extension/unittest
              extension/test_utilities)
  create_plato_unittester(PlatoFunctionalFilter "${FOLDERS}" PlatoFunctionalFilterTestUtilities)

  set(DIRECTORIES extension/parallel_unittest
                  extension/test_utilities)
  set(NUM_RANKS_FOR_TEST 4)
  create_plato_parallel_unittester(PlatoFunctionalFilter "${DIRECTORIES}" ${NUM_RANKS_FOR_TEST} PlatoFunctionalTPICommonTestUtilities)

endif()
