include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalInputValidation
              PlatoFunctionalProcessManager
              PlatoFunctionalDocumentation
              PlatoFunctionalServices)

create_plato_library(PlatoFunctional "." "${LINK_LIBS}")
target_include_directories(PlatoFunctional PRIVATE "${PLATO_BUILD_INFO_INCLUDE_DIR}")

if( UNIT_TESTING )
  # Build unit tester
  set(DIRECTORIES unittest)
  create_plato_unittester(PlatoFunctional "${DIRECTORIES}")
  target_include_directories(PlatoFunctional_UnitTester PRIVATE "${PLATO_BUILD_INFO_INCLUDE_DIR}")
endif()
