include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

create_plato_header_library(PlatoFunctionalTPICommon "." "")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalTPICommon unittest)
endif()
