include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

create_plato_header_library(PlatoFunctionalTPICommon "." "")
create_plato_interface(PlatoCommonInterface Vector3.hpp  "")

set(LINK_LIBS PlatoFunctionalTPICommon PlatoFunctionalTestUtilities)
create_plato_header_library(PlatoFunctionalTPICommonTestUtilities "test_utilities" "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalTPICommon unittest PlatoFunctionalTPICommonTestUtilities)
endif()
