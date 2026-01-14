include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

set(FOLDERS library
            extension
            extension/kernel_filters)

set(LINK_LIBS PlatoFunctionalCore
              PlatoFunctionalLinearAlgebra
              PlatoFunctionalMesh
              PlatoFunctionalServices
              PlatoFunctionalSTKIO
              PlatoFunctionalTPICommon
              PlatoFunctionalSTKSearch
              PlatoFunctionalTpetra
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalFilter "${FOLDERS}" "${LINK_LIBS}")
create_plato_interface(PlatoFilterInterface library/FilterInterface.hpp "")

if( UNIT_TESTING )
  # Build test utilities lib
  set(FOLDERS test_utilities
              extension/test_utilities
              extension/kernel_filters/test_utilities)
  create_plato_library(PlatoFunctionalFilterTestUtilities "${FOLDERS}" PlatoFunctionalFilter)

  # Build test filter shared libs
  create_plato_library(PlatoIdentityFilter test_identity_filter PlatoFunctionalFilter)
  create_plato_library(PlatoConstantValueFilter test_constant_value_filter PlatoFunctionalFilter)

  # Build unit tester
  set(FOLDERS library/unittest
              extension/unittest
              extension/kernel_filters/unittest)
  create_plato_unittester(PlatoFunctionalFilter "${FOLDERS}" PlatoFunctionalFilterTestUtilities PlatoFunctionalSTKIOTestUtilities)

  set(DIRECTORIES extension/kernel_filters/parallel_unittest
                  extension/test_utilities
                  extension/kernel_filters/test_utilities)
  set(NUM_RANKS_FOR_TEST 4)
  create_plato_parallel_unittester(PlatoFunctionalFilter "${DIRECTORIES}" ${NUM_RANKS_FOR_TEST} PlatoFunctionalTPICommonTestUtilities)

endif()
