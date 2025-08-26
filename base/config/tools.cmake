function( Plato_no_src_build )

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(STATUS " ")
  message(STATUS "In-source builds are not allowed.")
  message(STATUS "Please remove CMakeCache.txt and the CMakeFiles/ directory and then build out-of-source.")
  message(STATUS "(That is, create a build directory below the source directory and build from there.)" )
  message(STATUS " ")
  message(FATAL_ERROR " ")
endif()

endfunction( Plato_no_src_build )

###############################################################################
## Plato_add_test_files( 
##    FILE_LIST    == List of files to copy into build.
## )
###############################################################################

function( Plato_add_test_files FILE_LIST )
  
  foreach( testFile ${FILE_LIST} )
  
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${testFile} 
                   ${CMAKE_CURRENT_BINARY_DIR}/${testFile} COPYONLY)
    
  endforeach(testFile)
    
endfunction(Plato_add_test_files)
