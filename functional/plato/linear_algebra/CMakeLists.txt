include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

set(LINK_LIBS Boost::headers PlatoFunctionalUtilities)
create_plato_library(PlatoFunctionalLinearAlgebra "." "${LINK_LIBS}")
create_plato_interface(PlatoLinearAlgebraInterface DynamicVector.hpp "")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalLinearAlgebra unittest)
endif()
