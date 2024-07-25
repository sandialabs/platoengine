include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities
              PlatoFunctionalSTKIO)

create_plato_library(PlatoFunctionalMesh "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalMesh unittest PlatoFunctionalSTKIOTestUtilities)
endif()
