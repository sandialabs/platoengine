include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities STKSearch::stk_search PlatoFunctionalTPICommon)
create_plato_library(PlatoFunctionalSTKSearch "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalSTKSearch unittest)
endif()
