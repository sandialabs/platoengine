include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalInputValidation
              PlatoFunctionalProcessManager
              PlatoFunctionalServices)

create_plato_library(PlatoFunctional "." "${LINK_LIBS}")

if( UNIT_TESTING )
  # Build unit tester
  set(DIRECTORIES unittest)
  create_plato_unittester(PlatoFunctional "${DIRECTORIES}")
endif()
