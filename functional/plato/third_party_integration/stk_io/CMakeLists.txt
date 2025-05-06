include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities STKIO::stk_io PlatoFunctionalTPICommon)
create_plato_library(PlatoFunctionalSTKIO "." "${LINK_LIBS}")

if( UNIT_TESTING )
  set(LINK_LIBS PlatoFunctionalSTKIO PlatoFunctionalTestUtilities)
  create_plato_library(PlatoFunctionalSTKIOTestUtilities "test_utilities" "${LINK_LIBS}")
  create_plato_unittester(
    PlatoFunctionalSTKIO unittest PlatoFunctionalSTKIOTestUtilities PlatoFunctionalTPICommonTestUtilities)
endif()
