include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)

set(LINK_LIBS GTest::GTest
              Kokkos::kokkos 
              PlatoFunctionalInputParser)

create_plato_library(PlatoFunctionalTestUtilities "." "${LINK_LIBS}")