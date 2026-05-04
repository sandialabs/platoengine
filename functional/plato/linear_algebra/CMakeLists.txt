include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

set(LINK_LIBS Boost::headers PlatoFunctionalUtilities)
create_plato_library(PlatoFunctionalLinearAlgebra "." "${LINK_LIBS}")
create_plato_interface(PlatoLinearAlgebraInterface DynamicVector.hpp "")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalLinearAlgebra unittest)

  set(NUM_RANKS_FOR_TEST 4)
  create_plato_parallel_unittester(PlatoFunctionalLinearAlgebra "parallel_unittest" ${NUM_RANKS_FOR_TEST})
endif()
