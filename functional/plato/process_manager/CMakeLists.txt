include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(DIRECTORIES library extension)
set(LINK_LIBS PlatoFunctionalGeometry 
              PlatoFunctionalROL)

create_plato_library(PlatoFunctionalProcessManager "${DIRECTORIES}" "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build unit tester
  set(DIRECTORIES library/unittest extension/unittest)
  create_plato_unittester(PlatoFunctionalProcessManager "${DIRECTORIES}")
endif()
