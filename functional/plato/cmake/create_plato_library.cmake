include(${CMAKE_UTIL_DIR}/add_to_srcs_and_hdrs.cmake)

# create_plato_library 
#   LIBRARY_NAME: The name of the library and CMake target that will be created.
#   DIRECTORIES: A list of directories containing the test source files.
#   TARGET_LINK_LIST: A list of targets to link against.
function( create_plato_library LIBRARY_NAME DIRECTORIES TARGET_LINK_LIST)

    unset(LIB_SRCS)
    unset(LIB_HDRS)

    foreach( curDir ${DIRECTORIES} )
        add_to_srcs_and_hdrs(${curDir} LIB_SRCS LIB_HDRS)
    endforeach(curDir)

    add_library(${LIBRARY_NAME} SHARED ${LIB_SRCS} ${LIB_HDRS} )
    target_include_directories(${LIBRARY_NAME} PUBLIC $<BUILD_INTERFACE:${FUNCTIONAL_BASE_DIR}>)

    target_link_libraries(${LIBRARY_NAME} PUBLIC ${TARGET_LINK_LIST})

    install( TARGETS ${LIBRARY_NAME} EXPORT PlatoEngine
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME FUNCTIONAL_SUB_DIR)
    target_include_directories(${LIBRARY_NAME} INTERFACE $<INSTALL_INTERFACE:include/plato/${FUNCTIONAL_SUB_DIR}>)
    install( FILES ${LIB_HDRS} DESTINATION include/plato/${FUNCTIONAL_SUB_DIR})

endfunction(create_plato_library)