include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Krino::krino_lib PlatoFunctionalUtilities)
create_plato_library(PlatoFunctionalKrino "." "${LINK_LIBS}")

if( UNIT_TESTING )
  set(LINK_LIBS PlatoFunctionalKrino PlatoFunctionalTestUtilities)
  create_plato_library(PlatoFunctionalKrinoTestUtilities "./test_utilities" "${LINK_LIBS}")
  create_plato_unittester(PlatoFunctionalKrino "unittest" "PlatoFunctionalSTKIOTestUtilities;PlatoFunctionalKrinoTestUtilities")
endif()
