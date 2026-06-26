# add_documentation 
#   Copies all the markdown (*.md) files from ${DOCUMENTATION_PATH} to the build and install paths.
macro( add_documentation DOCUMENTATION_PATH)

    file(MAKE_DIRECTORY ${FUNCTIONAL_DATA_BUILD_PATH}/documentation)
    file(GLOB COPY_FILES ${DOCUMENTATION_PATH}/*.md)
    file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_BUILD_PATH}/documentation)
    file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_INSTALL_PATH}/documentation)

endmacro( add_documentation )
