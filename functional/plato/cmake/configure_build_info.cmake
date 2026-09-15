# configure_git_commit_hash
#
# Sets up the git commit hash as a cmake variable
#
# Sets:
#   PLATO_GIT_COMMIT_HASH
macro(configure_git_commit_hash)

    set(PLATO_GIT_COMMIT_HASH "")
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            OUTPUT_VARIABLE PLATO_GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE PLATO_GIT_COMMIT_RESULT
        )

        if(NOT PLATO_GIT_COMMIT_RESULT EQUAL 0)
            set(PLATO_GIT_COMMIT_HASH "")
        endif()
    endif()

endmacro(configure_git_commit_hash)

# configure_build_info
#
# Generates a doc_build_info.py file containing:
#   - project version
#   - git commit hash
#   - documentation build timestamp
# Generates BuildInfo.hpp in main/library defining:
#   - project version
#   - git commit hash
#
# Outputs:
#   documentation/build_info.py in both build and install docs paths
#   main/library/BuildInfo.hpp in build path
macro(configure_build_info)
    configure_git_commit_hash()
    set(PLATO_BUILD_INFO_INCLUDE_DIR "${PROJECT_BINARY_DIR}/functional")
    configure_file(
        "${CMAKE_UTIL_DIR}/BuildInfo.hpp.in"
        "${PLATO_BUILD_INFO_INCLUDE_DIR}/plato/main/library/BuildInfo.hpp"
        @ONLY
    )

    string(TIMESTAMP PLATO_DOC_BUILD_DATE "%Y-%m-%d %H:%M UTC" UTC)
    file(MAKE_DIRECTORY "${FUNCTIONAL_DATA_BUILD_PATH}/documentation")
    file(MAKE_DIRECTORY "${FUNCTIONAL_DATA_INSTALL_PATH}/documentation")

    configure_file("${CMAKE_UTIL_DIR}/build_info.py.in" "${FUNCTIONAL_DATA_BUILD_PATH}/documentation/build_info.py" @ONLY)
    configure_file("${CMAKE_UTIL_DIR}/build_info.py.in" "${FUNCTIONAL_DATA_INSTALL_PATH}/documentation/build_info.py" @ONLY)

endmacro(configure_build_info)
