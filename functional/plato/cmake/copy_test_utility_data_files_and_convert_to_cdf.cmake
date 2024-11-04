# copy_test_utility_data_files_and_convert_to_cdf
#   Copies all the files from ${FUNCTIONAL_TEST_DATA_DIR} to the build and install paths.
#   Runs ncgen on each file to convert them from git friendly ascii txt files of the meshes to binary cdf. 
macro( copy_test_utility_data_files_and_convert_to_cdf )
    find_program(NCGEN_EXE ncgen REQUIRED)

    file(GLOB COPY_FILES ${FUNCTIONAL_TEST_DATA_DIR}/*.txt)
    file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_BUILD_PATH})
    file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_INSTALL_PATH})

    foreach( TXTFILE ${COPY_FILES} )
        
        get_filename_component(BASE_FILE_NAME ${TXTFILE} NAME_WLE)
        set(FULLPATH_TXT_FILE "${FUNCTIONAL_TEST_DATA_DIR}/${BASE_FILE_NAME}.txt")
        set(BUILDPATH_CDF_FILE "${FUNCTIONAL_DATA_BUILD_PATH}/${BASE_FILE_NAME}.cdf")
        set(INSTALLPATH_CDF_FILE "${FUNCTIONAL_DATA_INSTALL_PATH}/${BASE_FILE_NAME}.cdf")
        
        execute_process(COMMAND "${NCGEN_EXE}" ${FULLPATH_TXT_FILE} -o ${BUILDPATH_CDF_FILE} ) 
        execute_process(COMMAND "${NCGEN_EXE}" ${FULLPATH_TXT_FILE} -o ${INSTALLPATH_CDF_FILE} ) 
        
    endforeach(TXTFILE)

endmacro(copy_test_utility_data_files_and_convert_to_cdf)
