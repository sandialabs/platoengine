include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(FOLDERS library
            extension)

set(LINK_LIBS PlatoFunctionalCore
              PlatoFunctionalLinearAlgebra)

create_plato_library(PlatoFunctionalFilter "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build test filter shared lib
  create_plato_library(PlatoIdentityFilter test_utilities PlatoFunctionalFilter)

  # Build unit tester
  set(FOLDERS library/unittest
              extension/unittest)
  create_plato_unittester(PlatoFunctionalFilter "${FOLDERS}")

endif()