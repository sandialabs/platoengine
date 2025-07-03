include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities)

create_plato_header_library(PlatoFunctionalComponents "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalComponents "unittest")
endif()