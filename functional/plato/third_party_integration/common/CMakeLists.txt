include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

create_plato_header_library(PlatoFunctionalTPICommon "." "")

set(LINK_LIBS PlatoFunctionalTPICommon PlatoFunctionalTestUtilities)
create_plato_header_library(PlatoFunctionalTPICommonTestUtilities "test_utilities" "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalTPICommon unittest PlatoFunctionalTPICommonTestUtilities)
endif()
