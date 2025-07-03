############ Project setup ##################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
#############################################################################

set (CMAKE_CXX_STANDARD 20)

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

###############################################################################
###############################################################################

project(PlatoEngine LANGUAGES CXX C)

find_package(MPI REQUIRED COMPONENTS CXX)

############################  Trilinos Setup  #################################
###############################################################################
IF (DEFINED TRILINOS_INSTALL_DIR)
  SET(CMAKE_PREFIX_PATH ${TRILINOS_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
ENDIF()
FIND_PACKAGE(Trilinos REQUIRED)

MESSAGE("\nFound Trilinos!  Here are the details: ")
MESSAGE("   Trilinos_DIR = ${Trilinos_DIR}")
MESSAGE("   Trilinos_VERSION = ${Trilinos_VERSION}")
MESSAGE("End of Trilinos details\n")

###########################  Compiler setup ###################################
message("Trilinos_CXX_COMPILER     ${Trilinos_CXX_COMPILER}")
message("Trilinos_C_COMPILER       ${Trilinos_C_COMPILER}")
message("Trilinos_Fortran_COMPILER ${Trilinos_Fortran_COMPILER}")
set(CMAKE_CXX_COMPILER     ${Trilinos_CXX_COMPILER})
set(CMAKE_C_COMPILER       ${Trilinos_C_COMPILER})
set(CMAKE_Fortran_COMPILER ${Trilinos_Fortran_COMPILER})

if(PLATOENGINE_ENABLE_CUDA)
  message(STATUS "Enabling CUDA")
  enable_language(CUDA)
endif()


########################### Compiler commands setup ###########################

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  file(CREATE_LINK "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}/compile_commands.json" SYMBOLIC )
endif()

###############################################################################
###############################################################################


find_package(Boost REQUIRED COMPONENTS serialization filesystem system program_options regex mpi log)
if( LINK_WITH_PYTHON )
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
endif()

###########################  Compiler setup ###################################
####################### (post 'project()' command) ############################
set(GCC_ADDITIONAL_WARNINGS -Wall 
                            -Wextra
                            -Wshadow
                            -Wnon-virtual-dtor
                            -Wsuggest-override
                            -pedantic
                            -Wno-return-type-c-linkage
                            -Wno-unknown-warning-option
                            -Wno-error=cpp)

if(BUILD_WITH_SANITIZER_FLAGS)
  message(STATUS "Compiling with sanitizer flags turned on")
  set(SANITIZER_FLAGS -fsanitize=address 
                      -fno-omit-frame-pointer 
                      -fsanitize=undefined 
                      -fno-sanitize-recover=all 
                      -fsanitize=float-divide-by-zero 
                      -fsanitize=float-cast-overflow  
                      -fsanitize=null 
                      -fno-sanitize=alignment
                      -fsanitize=enum)
  message(STATUS "Sanitizer flags: ${SANITIZER_FLAGS}")
  add_compile_options(${SANITIZER_FLAGS})
  add_link_options(${SANITIZER_FLAGS})
endif()

message("Trilinos_CXX_COMPILER_FLAGS: ${Trilinos_CXX_COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${Trilinos_C_COMPILER_FLAGS} -fPIC -Werror")
set(CMAKE_CXX_FLAGS "${Trilinos_CXX_COMPILER_FLAGS} -fPIC -Werror ")
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${GCC_ADDITIONAL_WARNINGS}>")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")


############################## gtest Setup ####################################
###############################################################################
if( UNIT_TESTING )
  find_package(GTest REQUIRED)
endif()

###############################################################################
###############################################################################
if("${CMAKE_BUILD_TYPE}" STREQUAL Release)
  add_definitions( -DNDEBUG)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
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

if( REGRESSION )

  integration_tests_dir()
  
  if( SIERRA_TESTS_ENABLED )
      install_prebuilt_binaries()
      add_integration_test(sierra)
  endif()
  
  add_integration_test(su2_to_exodus)
  add_integration_test(support_structure)
  add_integration_test(prune_and_refine)
  
endif()
