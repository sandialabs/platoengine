include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)

set(LINK_LIBS GTest::GTest
              Kokkos::kokkos 
              PlatoFunctionalInputParser)

create_plato_library(PlatoFunctionalTestUtilities "." "${LINK_LIBS}")

if(UNIT_TESTING)
    set(NUM_RANKS_FOR_TEST 4)
    create_plato_parallel_unittester(PlatoFunctionalTestUtilities "parallel_unittest" ${NUM_RANKS_FOR_TEST})
endif()
