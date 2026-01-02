macro(clang_tidy_setup)
  if( BUILD_WITH_CLANG_TIDY )
    find_program(CLANGTIDY clang-tidy NAMES clang-tidy clang-tidy-18)
    if(NOT CLANGTIDY)
      message(FATAL_ERROR "Requested to build with clang-tidy, but could not find the executable. Check your path.")
    endif()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CLANG_TIDY_EXTRA_ARGS "")

    if(DEFINED ENV{SPACK_TARGET_ARGS_CXX})
      # Add any spack CPU target flags that may conflict with generated PCH
      set(SPACK_TARGET_ARGS_CXX $ENV{SPACK_TARGET_ARGS_CXX})
      string(REPLACE " " ";" SPACK_TARGET_ARGS_LIST "${SPACK_TARGET_ARGS_CXX}")
      foreach(FLAG ${SPACK_TARGET_ARGS_LIST})
        list(APPEND CLANG_TIDY_EXTRA_ARGS "--extra-arg=${FLAG}")
      endforeach()
    endif()

    if(GCC_TOOLCHAIN_PATH)
      list(APPEND CLANG_TIDY_EXTRA_ARGS "--extra-arg=--gcc-toolchain=${GCC_TOOLCHAIN_PATH}")
    endif()

    set(CLANG_TIDY_COMMAND ${CLANGTIDY} ${CLANG_TIDY_EXTRA_ARGS})

    message(STATUS "Using clang-tidy")
    message(STATUS "Clang-tidy command: ${CLANG_TIDY_COMMAND}")
  endif()
endmacro(clang_tidy_setup)
