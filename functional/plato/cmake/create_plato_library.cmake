include(${CMAKE_UTIL_DIR}/add_to_srcs_and_hdrs.cmake)

# create_plato_header_library 
#  This version must be used if a library only contains header files.
#   LIBRARY_NAME: The name of the library and CMake target that will be created.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: A list of targets to link against.
function( create_plato_header_library LIBRARY_NAME DIRECTORIES TARGET_LINK_LIST)
    set(LIB_TYPE INTERFACE)
    set(EXPORT_TYPE INTERFACE)
    create_plato_library_impl(${LIBRARY_NAME} "${DIRECTORIES}" "${TARGET_LINK_LIST}" ${LIB_TYPE} ${EXPORT_TYPE})
endfunction(create_plato_header_library)

# create_plato_library 
#   LIBRARY_NAME: The name of the library and CMake target that will be created.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: A list of targets to link against.
function( create_plato_library LIBRARY_NAME DIRECTORIES TARGET_LINK_LIST)
    set(LIB_TYPE SHARED)
    set(EXPORT_TYPE PUBLIC)
    create_plato_library_impl(${LIBRARY_NAME} "${DIRECTORIES}" "${TARGET_LINK_LIST}" ${LIB_TYPE} ${EXPORT_TYPE})
endfunction(create_plato_library)

# Implementation detail
function(create_plato_library_impl LIBRARY_NAME DIRECTORIES TARGET_LINK_LIST LIB_TYPE EXPORT_TYPE)

    unset(LIB_SRCS)
    unset(LIB_HDRS)

    foreach( curDir ${DIRECTORIES} )
        add_to_srcs_and_hdrs(${curDir} LIB_SRCS LIB_HDRS)
    endforeach(curDir)

    add_library(${LIBRARY_NAME} ${LIB_TYPE} ${LIB_SRCS} ${LIB_HDRS} )
    target_include_directories(${LIBRARY_NAME} ${EXPORT_TYPE} $<BUILD_INTERFACE:${FUNCTIONAL_BASE_DIR}>)

    target_link_libraries(${LIBRARY_NAME} ${EXPORT_TYPE} ${TARGET_LINK_LIST})
    if(BUILD_WITH_CLANG_TIDY)
        set_target_properties(${LIBRARY_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    endif()

    install( TARGETS ${LIBRARY_NAME} EXPORT PlatoEngine
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME FUNCTIONAL_SUB_DIR)
    target_include_directories(${LIBRARY_NAME} INTERFACE $<INSTALL_INTERFACE:include/>)
    install( FILES ${LIB_HDRS} DESTINATION include/plato/${FUNCTIONAL_SUB_DIR})

endfunction()
