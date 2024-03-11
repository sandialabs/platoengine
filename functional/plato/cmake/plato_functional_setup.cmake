macro(plato_functional_setup)
    set(TEST_UNIT_MAIN_INCL ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TestMain.cpp)
    set(PARALLEL_TEST_UNIT_MAIN_INCL ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ParallelUnitMain.cpp)
    set(CMAKE_UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    set(FUNCTIONAL_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
endmacro(plato_functional_setup)
