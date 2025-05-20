macro(plato_functional_setup)
    set(TEST_UNIT_MAIN_INCL ${CMAKE_CURRENT_SOURCE_DIR}/cmake/TestMain.cpp)
    set(PARALLEL_TEST_UNIT_MAIN_INCL ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ParallelUnitMain.cpp)
    set(CMAKE_UTIL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    set(FUNCTIONAL_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
    set(PLATO_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(FUNCTIONAL_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_utilities/data/)
    set(FUNCTIONAL_PLUGIN_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/plugins")
endmacro(plato_functional_setup)

macro(create_coverage_target)
    add_library(CoverageInterface INTERFACE)
    target_compile_options(CoverageInterface INTERFACE $<$<CONFIG:Debug>:--coverage>)
    target_link_options(CoverageInterface INTERFACE $<$<CONFIG:Debug>:--coverage>)
    install( TARGETS CoverageInterface EXPORT PlatoEngine
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endmacro(create_coverage_target)
