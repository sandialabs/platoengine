# copy_rol_data
#   Creates a variable to refer to in the code FUNCTIONAL_ROL_DATA_DIR.
#   Copies the rol_inputs.xml file from ${FUNCTIONAL_ROL_DATA_DIR} to the build and install paths.
macro( copy_rol_data )

    file(GLOB COPY_FILES ${FUNCTIONAL_ROL_DATA_DIR}/*.xml)
    file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_BUILD_PATH})
    file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_INSTALL_PATH})

endmacro(copy_rol_data)
