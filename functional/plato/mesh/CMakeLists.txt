include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities
              PlatoFunctionalAnalysis
              PlatoFunctionalSTKIO
              PlatoFunctionalKrino)

create_plato_library(PlatoFunctionalMesh "." "${LINK_LIBS}")

if( UNIT_TESTING )
  set(LINK_LIBS PlatoFunctionalSTKIOTestUtilities)
  create_plato_unittester(PlatoFunctionalMesh unittest "${LINK_LIBS}")
endif()
