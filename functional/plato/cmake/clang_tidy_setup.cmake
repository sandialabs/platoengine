macro(clang_tidy_setup)
  if( BUILD_WITH_CLANG_TIDY )
    find_program(CLANGTIDY clang-tidy NAMES clang-tidy clang-tidy-18)
    if(NOT CLANGTIDY)
      message(FATAL_ERROR "Requested to build with clang-tidy, but could not find the executable. Check your path.")
    endif()
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CLANG_TIDY_EXTRA_ARCH_ARGS "")
    if(DEFINED ENV{SPACK_TARGET_ARGS_CXX})
      # Add any spack CPU target flags that may conflict with generated PCH files
      set(SPACK_TARGET_ARGS_CXX $ENV{SPACK_TARGET_ARGS_CXX})
      string(REPLACE " " ";" SPACK_TARGET_ARGS_LIST "${SPACK_TARGET_ARGS_CXX}")
      foreach(FLAG ${SPACK_TARGET_ARGS_LIST})
        list(APPEND CLANG_TIDY_EXTRA_ARCH_ARGS "--extra-arg=${FLAG}")
      endforeach()
    endif()

    set(CLANG_TIDY_EXTRA_ARGS "")
    if(GCC_TOOLCHAIN_PATH)
      list(APPEND CLANG_TIDY_EXTRA_ARGS "--extra-arg=--gcc-toolchain=${GCC_TOOLCHAIN_PATH}")
    endif()

    set(CLANG_TIDY_COMMAND ${CLANGTIDY} ${CLANG_TIDY_EXTRA_ARGS})

    message(STATUS "Using clang-tidy")
    message(STATUS "Clang-tidy command: ${CLANG_TIDY_COMMAND}")
  endif()
endmacro(clang_tidy_setup)

# targets_with_arch_flags: Searches LIBRARY_TARGET and its dependencies for any target that uses march or mtune flags.
# A list of such targets is generated in TARGETS_WITH_ARCH_FLAGS_OUT.
#
# The purpose of this is to fix issues with clang-tidy, PCH, and spack/kokkos setting of march/mtune flags. Spack sets 
# march/mtune flags via an environment variable, but kokkos may set those flags differently. So we can't just set all targets
# to use Spack's march/mtune flag since it may conflict with kokkos.
function(targets_with_arch_flags LIBRARY_TARGET TARGETS_WITH_ARCH_FLAGS_OUT)

  set(VISITED_DEPENDENCIES "" CACHE INTERNAL "")
  set(ARCH_LIBS "" CACHE INTERNAL "")

  targets_with_arch_flags_impl("${LIBRARY_TARGET}" VISITED_DEPENDENCIES ARCH_LIBS)

  set(${TARGETS_WITH_ARCH_FLAGS_OUT} "$CACHE{ARCH_LIBS}" PARENT_SCOPE)

endfunction()

function(targets_with_arch_flags_impl CURRENT_TARGET VISITED_INOUT ARCH_LIBS_INOUT)

  set(VISITED "$CACHE{${VISITED_INOUT}}")
  list(FIND VISITED "${CURRENT_TARGET}" INDEX)
  if(NOT INDEX EQUAL -1)
    # This dependency has been visited already
    return()
  endif()

  list(APPEND VISITED "${CURRENT_TARGET}")
  set(${VISITED_INOUT} "${VISITED}" CACHE INTERNAL "")

  # Check the current target's interface options
  get_target_property(CURRENT_COMPILE_OPTIONS "${CURRENT_TARGET}" INTERFACE_COMPILE_OPTIONS)
  set(CURRENT_ARCH_LIBS "$CACHE{${ARCH_LIBS_INOUT}}")
  if(NOT CURRENT_COMPILE_OPTIONS STREQUAL "NOTFOUND")
    foreach(CURRENT_OPTION IN LISTS CURRENT_COMPILE_OPTIONS)
      if(CURRENT_OPTION MATCHES "^-march=[^ ]+" OR CURRENT_OPTION MATCHES "^-mtune=[^ ]+")
        list(APPEND CURRENT_ARCH_LIBS "${CURRENT_TARGET}")
      endif()
    endforeach()
    set(${ARCH_LIBS_INOUT} "${CURRENT_ARCH_LIBS}" CACHE INTERNAL "")
  endif()

  # Recurse through the dependencies
  get_target_property(CURRENT_DEPENDENCIES "${CURRENT_TARGET}" INTERFACE_LINK_LIBRARIES)
  if(NOT CURRENT_DEPENDENCIES STREQUAL "NOTFOUND")
    foreach(CURRENT_DEPENDENCY IN LISTS CURRENT_DEPENDENCIES)
      if(TARGET "${CURRENT_DEPENDENCY}")
        targets_with_arch_flags_impl("${CURRENT_DEPENDENCY}" "${VISITED_INOUT}" "${ARCH_LIBS_INOUT}")
      endif()
    endforeach()
  endif()
endfunction()
