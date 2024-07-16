include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Boost::headers Boost::mpi Boost::filesystem)
create_plato_library(PlatoFunctionalUtilities "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalUtilities unittest)
endif()
