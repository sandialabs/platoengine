# create_data_paths
#   Creates two variables to refer to in the code FUNCTIONAL_DATA_BUILD_PATH and FUNCTIONAL_DATA_INSTALL_PATH
#   Make the directories for these paths
macro( create_data_paths )

    set( FUNCTIONAL_DATA_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/data")
    set( FUNCTIONAL_DATA_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/data")

    file(MAKE_DIRECTORY ${FUNCTIONAL_DATA_BUILD_PATH})
    file(MAKE_DIRECTORY ${FUNCTIONAL_DATA_INSTALL_PATH})

endmacro(create_data_paths)
