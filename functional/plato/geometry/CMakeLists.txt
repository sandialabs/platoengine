include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(FOLDERS library 
            extension)
set(LINK_LIBS PlatoFunctionalFilter )

create_plato_library(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build unit tester
  set(FOLDERS library/unittest 
              extension/unittest)
  create_plato_unittester(PlatoFunctionalGeometry "${FOLDERS}")

endif()