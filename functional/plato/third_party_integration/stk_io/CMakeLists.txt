include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities STKIO::stk_io PlatoFunctionalTPICommon)
create_plato_library(PlatoFunctionalSTKIO "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalSTKIO unittest)
endif()
