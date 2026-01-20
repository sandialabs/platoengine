include(${CMAKE_UTIL_DIR}/add_to_srcs_and_hdrs.cmake)

# create_plato_unittester
#   TEST_LIB: The name of the library to create a unit tester for. The test exe name will be
#    the name of the library with `_UnitTester` appended.
#   DIRECTORIES: A list of directories containing the test source files.
#   Additional targets to link with may be passed as extra arguments.
function( create_plato_unittester TEST_LIB DIRECTORIES )

    set(EXTRA_LIBS ${ARGN})
    set(TEST_EXE "${TEST_LIB}_UnitTester")
    set(TARGET_LINK_LIST "${TEST_LIB}")
    set(NUM_RANKS_FOR_TEST "1")
    create_plato_unittester_impl( ${TEST_EXE} "${DIRECTORIES}" ${TEST_UNIT_MAIN_INCL} "${TARGET_LINK_LIST};${EXTRA_LIBS}" ${NUM_RANKS_FOR_TEST} )

endfunction(create_plato_unittester)

# create_plato_integration_tester
#   TEST_EXE: The name of the test executable and CMake target.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: List of targets to link with.
function( create_plato_integration_tester TEST_EXE DIRECTORIES TARGET_LINK_LIST)

    set(NUM_RANKS_FOR_TEST "1")
    create_plato_unittester_impl( ${TEST_EXE} "${DIRECTORIES}" ${TEST_UNIT_MAIN_INCL} "${TARGET_LINK_LIST}" ${NUM_RANKS_FOR_TEST})

endfunction(create_plato_integration_tester)

# create_plato_parallel_unittester
#   TEST_LIB: The name of the library to create a unit tester for. The test exe name will be
#    the name of the library with `_ParallelUnitTester` appended.
#   DIRECTORIES: A list of directories containing the test source files.
#   NUM_RANKS_FOR_TEST: The number of MPI ranks to use in the test executable.
#   Additional targets to link with may be passed as extra arguments.
function( create_plato_parallel_unittester TEST_LIB DIRECTORIES NUM_RANKS_FOR_TEST)

    set(EXTRA_LIBS ${ARGN})
    set(TEST_EXE "${TEST_LIB}_ParallelUnitTester")
    set(TARGET_LINK_LIST "${TEST_LIB}")
    create_plato_parallel_integration_tester( ${TEST_EXE} "${DIRECTORIES}" "${TARGET_LINK_LIST};${EXTRA_LIBS}" ${NUM_RANKS_FOR_TEST})

endfunction(create_plato_parallel_unittester)

# create_plato_parallel_integration_tester
#   TEST_EXE: The name of the test executable and CMake target.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: List of targets to link with.
#   NUM_RANKS_FOR_TEST: The number of MPI ranks to use in the test executable.
function( create_plato_parallel_integration_tester TEST_EXE DIRECTORIES TARGET_LINK_LIST NUM_RANKS_FOR_TEST)

    configure_file(${PARALLEL_TEST_UNIT_MAIN_INCL} ${CMAKE_CURRENT_BINARY_DIR}/ParallelUnitMain.cpp)
    set( TEST_MAIN_CPP "${CMAKE_CURRENT_BINARY_DIR}/ParallelUnitMain.cpp" )
    create_plato_unittester_impl( ${TEST_EXE} "${DIRECTORIES}" ${TEST_MAIN_CPP} "${TARGET_LINK_LIST}" ${NUM_RANKS_FOR_TEST})

endfunction(create_plato_parallel_integration_tester)

function( create_plato_unittester_impl TEST_EXE DIRECTORIES TEST_MAIN_CPP TARGET_LINK_LIST NUM_RANKS_FOR_TEST)

    unset(TEST_SRCS)
    unset(TEST_HDRS)

    foreach( curDir ${DIRECTORIES} )
        add_to_srcs_and_hdrs(${curDir} TEST_SRCS TEST_HDRS)
    endforeach(curDir)
    
    list(APPEND TEST_SRCS ${TEST_MAIN_CPP})

    add_executable(${TEST_EXE} ${TEST_SRCS} ${TEST_HDRS})
    add_dependencies(${TEST_EXE} clean_coverage)
    target_compile_options(${TEST_EXE} PRIVATE "-fPIC")

    target_link_libraries( ${TEST_EXE} PRIVATE GTest::GTest PlatoFunctionalTestUtilities CoverageInterface UnitTestPrecompiledHeaderInterface ${TARGET_LINK_LIST})
    add_test(NAME ${TEST_EXE} COMMAND ${TEST_EXE} --gtest_output=xml:${TEST_EXE}.xml)

    set( NUM_THREADS "1" )
    if( OPENMP_ENABLED )
        set( NUM_THREADS "2" )
    endif()
    math(EXPR NUM_PROCESSORS "${NUM_RANKS_FOR_TEST} * ${NUM_THREADS}")
    set_tests_properties( ${TEST_EXE} PROPERTIES LABELS "small" PROCESSORS ${NUM_PROCESSORS} ENVIRONMENT "OMP_NUM_THREADS=${NUM_THREADS};OMP_PROC_BIND=false;OMP_PLACES=threads")

    install( TARGETS ${TEST_EXE} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )

endfunction(create_plato_unittester_impl)
