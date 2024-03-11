include(${CMAKE_UTIL_DIR}/add_to_srcs_and_hdrs.cmake)

# create_plato_unittester
#   TEST_LIB: The name of the library to create a unit tester for. The test exe name will be
#    the name of the library with `_UnitTester` appended.
#   DIRECTORIES: A list of directories containing the test source files.
function( create_plato_unittester TEST_LIB DIRECTORIES)

    set(TEST_EXE "${TEST_LIB}_UnitTester")
    set(TARGET_LINK_LIST "${TEST_LIB}")
    create_plato_unittester_impl( ${TEST_EXE} "${DIRECTORIES}" ${TEST_UNIT_MAIN_INCL} "${TARGET_LINK_LIST}" )

endfunction(create_plato_unittester)

# create_plato_integration_tester
#   TEST_EXE: The name of the test executable and CMake target.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: List of targets to link with.
function( create_plato_integration_tester TEST_EXE DIRECTORIES TARGET_LINK_LIST)

    create_plato_unittester_impl( ${TEST_EXE} "${DIRECTORIES}" ${TEST_UNIT_MAIN_INCL} "${TARGET_LINK_LIST}" )

endfunction(create_plato_integration_tester)

# create_plato_parallel_unittester
#   TEST_LIB: The name of the library to create a unit tester for. The test exe name will be
#    the name of the library with `_ParallelUnitTester` appended.
#   DIRECTORIES: A list of directories containing the test source files.
#   NUM_RANKS_FOR_TEST: The number of MPI ranks to use in the test executable.
function( create_plato_parallel_unittester TEST_LIB DIRECTORIES NUM_RANKS_FOR_TEST)

    set(TEST_EXE "${TEST_LIB}_ParallelUnitTester")
    set(TARGET_LINK_LIST "${TEST_LIB}")
    create_plato_parallel_integration_tester( ${TEST_EXE} "${DIRECTORIES}" "${TARGET_LINK_LIST}" ${NUM_RANKS_FOR_TEST})

endfunction(create_plato_parallel_unittester)

# create_plato_parallel_integration_tester
#   TEST_EXE: The name of the test executable and CMake target.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: List of targets to link with.
#   NUM_RANKS_FOR_TEST: The number of MPI ranks to use in the test executable.
function( create_plato_parallel_integration_tester TEST_EXE DIRECTORIES TARGET_LINK_LIST NUM_RANKS_FOR_TEST)

    configure_file(${PARALLEL_TEST_UNIT_MAIN_INCL} ${CMAKE_CURRENT_BINARY_DIR}/ParallelUnitMain.cpp)
    set( TEST_MAIN_CPP "${CMAKE_CURRENT_BINARY_DIR}/ParallelUnitMain.cpp" )
    create_plato_unittester_impl( ${TEST_EXE} "${DIRECTORIES}" ${TEST_MAIN_CPP} "${TARGET_LINK_LIST}" )
    set_property(TEST ${TEST_EXE} PROPERTY PROCESSORS ${NUM_RANKS_FOR_TEST})

endfunction(create_plato_parallel_integration_tester)

function( create_plato_unittester_impl TEST_EXE DIRECTORIES TEST_MAIN_CPP TARGET_LINK_LIST)

    unset(TEST_SRCS)
    unset(TEST_HDRS)

    foreach( curDir ${DIRECTORIES} )
        add_to_srcs_and_hdrs(${curDir} TEST_SRCS TEST_HDRS)
    endforeach(curDir)
    
    list(APPEND TEST_SRCS ${TEST_MAIN_CPP})

    add_executable(${TEST_EXE} ${TEST_SRCS} ${TEST_HDRS})
    target_link_libraries( ${TEST_EXE} PRIVATE GTest::GTest PlatoFunctionalTestUtilities ${TARGET_LINK_LIST})
    add_test(NAME ${TEST_EXE} COMMAND ${TEST_EXE})
    set_property(TEST ${TEST_EXE} PROPERTY LABELS "small")

    install( TARGETS ${TEST_EXE} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

endfunction(create_plato_unittester_impl)
