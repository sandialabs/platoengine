include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Boost::headers)
create_plato_library(PlatoFunctionalLinearAlgebra "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalLinearAlgebra unittest)
endif()
