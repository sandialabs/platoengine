###############################################################################
## Plato_find_lib( 
##    VAR_NAME      == Return variable containing filepath to library.
##    OPTION_NAME   == If ON, function attempts to find requested library.
##    LIB_BASE_NAME == Basename of library.
##    SEARCH_PATH   == Directories below this path are searched.
##   [PROPER_NAME]  == Alternate name.  Used for message output only.
## )
###############################################################################

function( Plato_find_lib VAR_NAME OPTION_NAME LIB_BASE_NAME SEARCH_PATH )

  if( ${OPTION_NAME} )

message(STATUS "The search path is:  ${SEARCH_PATH} ") 

  if( ARGN GREATER 0 )  ## if optional argument included
    set(OUT_NAME ${ARGV0})
  else( ARGN GREATER 0 )  ## otherwise
    set(OUT_NAME ${LIB_BASE_NAME})
  endif( ARGN GREATER 0 )
  
  message(STATUS " ")
  message(STATUS "Finding ${OUT_NAME} executable")
  
  find_library( ${VAR_NAME}_SEARCH_RESULT ${LIB_BASE_NAME}
                HINTS ${SEARCH_PATH}
                DOC "${OUT_NAME} library"
                NO_DEFAULT_PATH )
  
  if( ${VAR_NAME}_SEARCH_RESULT MATCHES "NOTFOUND" )
    message(FATAL_ERROR "!! ${OUT_NAME} library not found !!")
  endif( ${VAR_NAME}_SEARCH_RESULT MATCHES "NOTFOUND" )
  
  set( ${VAR_NAME} ${${VAR_NAME}_SEARCH_RESULT} PARENT_SCOPE )
  message(STATUS "${OUT_NAME} library found")
  message(STATUS "Using:  ${${VAR_NAME}_SEARCH_RESULT}")
  message(STATUS " ")
  
  endif( ${OPTION_NAME} )
    
endfunction(Plato_find_lib)

###############################################################################
## Plato_find_path( 
##    VAR_NAME      == Return variable containing filepath to file
##    OPTION_NAME   == If ON, function attempts to find requested file
##    FILE_NAME     == name of file to be found
##    SEARCH_PATH   == Directories below this path are searched.
## )
###############################################################################

function( Plato_find_path VAR_NAME OPTION_NAME FILE_NAME SEARCH_PATH )

  if( ${OPTION_NAME} )

message(STATUS "The search path is:  ${SEARCH_PATH} ") 

  message(STATUS " ")
  message(STATUS "Finding ${OUT_NAME}")
  
  find_path( ${VAR_NAME}_SEARCH_RESULT ${FILE_NAME}
                HINTS ${SEARCH_PATH}
                DOC "${FILE_NAME} file"
                NO_DEFAULT_PATH )
  
  if( ${VAR_NAME}_SEARCH_RESULT MATCHES "NOTFOUND" )
    message(FATAL_ERROR "!! ${FILE_NAME} not found !!")
  endif( ${VAR_NAME}_SEARCH_RESULT MATCHES "NOTFOUND" )
  
  set( ${VAR_NAME} ${${VAR_NAME}_SEARCH_RESULT} PARENT_SCOPE )
  message(STATUS "${FILE_NAME} found")
  message(STATUS "Using:  ${${VAR_NAME}_SEARCH_RESULT}")
  message(STATUS " ")
  
  endif( ${OPTION_NAME} )
    
endfunction(Plato_find_path)

###############################################################################
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

