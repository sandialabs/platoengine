include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/add_documentation.cmake)

set(LINK_LIBS PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalComponents "." "${LINK_LIBS}")
add_documentation("documentation")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalComponents "unittest")
endif()