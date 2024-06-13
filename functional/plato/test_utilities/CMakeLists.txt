include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)

set(LINK_LIBS GTest::GTest
              Kokkos::kokkos 
              PlatoFunctionalInputParser)

configure_file(${FUNCTIONAL_BASE_DIR}/plato/cmake/TestDataFilePath.cpp ${CMAKE_CURRENT_BINARY_DIR}/configured_src/TestDataFilePath.cpp)
create_plato_library(PlatoFunctionalTestUtilities ".;${CMAKE_CURRENT_BINARY_DIR}/configured_src" "${LINK_LIBS}")

if(UNIT_TESTING)
    set(NUM_RANKS_FOR_TEST 4)
    create_plato_parallel_unittester(PlatoFunctionalTestUtilities "parallel_unittest" ${NUM_RANKS_FOR_TEST})
endif()
