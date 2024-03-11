include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS ROL::rol
              PlatoFunctionalCore)
create_plato_library(PlatoFunctionalOptimizer "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalOptimizer unittest)
endif()
