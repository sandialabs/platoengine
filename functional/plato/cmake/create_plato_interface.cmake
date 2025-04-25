# create_plato_interface
#  Add a list of headers to the plato interface
#   LIBRARY_NAME: The name of the library and CMake target that will be created.
#   HEADERS: A list of headers to include in the interface
#   TARGET_LINK_LIST: A list of other interface library dependencies for this library
function( create_plato_interface LIBRARY_NAME HEADERS TARGET_LINK_LIST )
    
    add_library(${LIBRARY_NAME} INTERFACE)
    target_include_directories(${LIBRARY_NAME} INTERFACE $<INSTALL_INTERFACE:include/>)    
    target_link_libraries(${LIBRARY_NAME} INTERFACE ${TARGET_LINK_LIST})

    cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME FUNCTIONAL_SUB_DIR)
    foreach( currentHeader ${HEADERS} )
        file(RELATIVE_PATH RELATIVE_PATH_FROM_PLATO_WITH_HEADER "${PLATO_BASE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${currentHeader}")
        get_filename_component(RELATIVE_PATH_FROM_PLATO "${RELATIVE_PATH_FROM_PLATO_WITH_HEADER}" DIRECTORY)
        install( FILES ${currentHeader} DESTINATION include/plato/${RELATIVE_PATH_FROM_PLATO})
    endforeach(currentHeader)

    install( TARGETS ${LIBRARY_NAME} EXPORT PlatoEngine
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endfunction(create_plato_interface)
