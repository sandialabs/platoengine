############ Project setup ##################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
#############################################################################

set (CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0144 NEW) # Ignores BOOST_ROOT

include( ${CMAKE_SOURCE_DIR}/base/config/tools.cmake )
include( ${CMAKE_SOURCE_DIR}/base/config/prebuilt_binaries.cmake )
include( ${CMAKE_SOURCE_DIR}/base/config/integration_tests_dir.cmake )

Plato_no_src_build()

# Advertise that we support shared libs (cmake squawks otherwise)
SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

# Add ccache if available
find_program(CCACHE_FOUND ccache)
if( CCACHE_FOUND )
  # Spack is able to use ccache, however, using ccache with nvcc_wrapper does not seem to work
  # properly through spack.
  message( STATUS "Found ccache" )
  set( CMAKE_CXX_COMPILER_LAUNCHER ccache )
endif()

option( REGRESSION    "Flag to create regression test suite"               OFF )
option( UNIT_TESTING  "Flag to turn on unit testing"                       OFF )
option( WARNINGS_AS_ERRORS "Builds platoengine with warnings as errors"    OFF )
option( BUILD_WITH_CLANG_TIDY "Builds platoengine with clang-tidy"         OFF )
option( BUILD_WITH_SANITIZER_FLAGS "Builds platoengine with sanitizers"    OFF )

###############################################################################
###############################################################################

project(PlatoEngine LANGUAGES CXX C)

find_package(MPI REQUIRED COMPONENTS CXX)

if(PLATOENGINE_ENABLE_CUDA)
  message(STATUS "Enabling CUDA")
  enable_language(CUDA)
endif()

############################  External Setup  #################################
###############################################################################

find_package(Trilinos REQUIRED)

find_package(Boost CONFIG REQUIRED COMPONENTS serialization filesystem system program_options regex mpi log)

if( LINK_WITH_PYTHON )
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

########################### Compiler commands setup ###########################

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  file(CREATE_LINK "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}/compile_commands.json" SYMBOLIC )
endif()

###########################  Compiler setup ###################################

message(STATUS "Additional warnings: ${ADDITIONAL_WARNINGS}")
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${ADDITIONAL_WARNINGS}>")

if(WARNINGS_AS_ERRORS)
  message(STATUS "Using -Werror")
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Werror>")
endif()

if(BUILD_WITH_SANITIZER_FLAGS)
  message(STATUS "Compiling with sanitizer flags turned on")
  message(STATUS "Sanitizer flags: ${SANITIZER_FLAGS}")
  add_compile_options(${SANITIZER_FLAGS})
  add_link_options(${SANITIZER_FLAGS})
endif()

############################## Test Setup #####################################
###############################################################################

if( UNIT_TESTING )
  find_package(GTest REQUIRED)
endif()

if( REGRESSION OR UNIT_TESTING )
    enable_testing()
endif()

add_subdirectory(base)
add_subdirectory(functional)

include(CMakePackageConfigHelpers)
install(EXPORT PlatoEngine NAMESPACE PlatoEngine FILE PlatoEngineTargets.cmake DESTINATION cmake)
set(Plugins_install_dir "plugins")
configure_package_config_file(PlatoEngineConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PlatoEngineConfig.cmake PATH_VARS Plugins_install_dir INSTALL_DESTINATION cmake)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PlatoEngineConfig.cmake DESTINATION cmake)

install(EXPORT PlatoEngine NAMESPACE PlatoEngine:: FILE PlatoEngineTargets.cmake DESTINATION cmake)

install_prebuilt_binaries()

if( REGRESSION )

  integration_tests_dir()
  
  if( SIERRA_TESTS_ENABLED )
      add_integration_test(sierra)
  endif()
  
  add_integration_test(su2_to_exodus)
  add_integration_test(support_structure)
  add_integration_test(prune_and_refine)
  
endif()
