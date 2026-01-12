# clean_gcda
# This script runs at build-time and deletes all *.gcda files in the build directory
if(NOT DEFINED CMAKE_BINARY_DIR)
  message(FATAL_ERROR "CMAKE_BINARY_DIR is not defined")
endif()

file(GLOB_RECURSE GCDA_FILES "${CMAKE_BINARY_DIR}/*.gcda")
file(REMOVE ${GCDA_FILES})
