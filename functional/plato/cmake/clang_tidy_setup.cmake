macro(clang_tidy_setup)
  if( BUILD_WITH_CLANG_TIDY )
    find_program(CLANGTIDY clang-tidy NAMES clang-tidy clang-tidy-16)
    if(NOT CLANGTIDY)
      message(FATAL_ERROR "Requested to build with clang-tidy, but could not find the executable. Check your path.")
    endif()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    # It seems like MPI_CXX_INCLUDE_DIRS should work, but it's empty.
    # Use the parent path of the mpi compiler wrapper:
    cmake_path(GET MPI_CXX_COMPILER PARENT_PATH MPI_COMPILER_PARENT_PATH)
    cmake_path(GET MPI_COMPILER_PARENT_PATH PARENT_PATH MPI_PARENT_PATH)
    set(CLANG_TIDY_EXTRA_ARGS "-I${MPI_PARENT_PATH}/include")
    set(CLANG_TIDY_COMMAND ${CLANGTIDY} --extra-arg=${CLANG_TIDY_EXTRA_ARGS})

    message(STATUS "Using clang-tidy")
    message(STATUS "Clang-tidy command: ${CLANG_TIDY_COMMAND}")
  endif()
endmacro(clang_tidy_setup)
