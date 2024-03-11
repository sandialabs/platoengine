include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalGeometry 
              PlatoFunctionalOptimizer
              PlatoFunctionalROLIntegration)

create_plato_library(PlatoFunctional "." "${LINK_LIBS}")

if( UNIT_TESTING )
    create_plato_unittester(PlatoFunctional unittest)
endif()
