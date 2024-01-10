############ Project setup ##################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
#############################################################################

set (CMAKE_CXX_STANDARD 17)

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

##################################################################
## Load config.cmake                                            ##
##################################################################
if( DEFINED CONFIG )

  message(STATUS "-DCONFIG specified: " ${CONFIG})
  if( EXISTS ${CMAKE_BINARY_DIR}/${CONFIG} )
   include( ${CMAKE_BINARY_DIR}/${CONFIG} )
  else()
    message(FATAL_ERROR "The configuration file '${CONFIG}' specified with -DCONFIG doesn't exist")
  endif()

else()

  if( EXISTS ${CMAKE_BINARY_DIR}/config.cmake )
   include( ${CMAKE_BINARY_DIR}/config.cmake )
  else()
    message(STATUS "A configuration file named 'config.cmake' was not provided in the build directory.")
    message(STATUS "Using command line arguments only to configure build.")
    include( ${CMAKE_SOURCE_DIR}/base/config/defaults.cmake )
  endif()

endif()

if( DEFINED PLATO_SYSTEM_CONFIG )
  if( EXISTS ${PLATO_SYSTEM_CONFIG} )
   include( ${PLATO_SYSTEM_CONFIG} )
  else()
    message(STATUS "Specified system configuration file ($PLATO_SYSTEM_CONFIG} not found")
    message(FATAL_ERROR " ")
  endif()
endif()
###############################################################################
###############################################################################

project(PlatoEngine LANGUAGES CXX C)

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

###############################################################################
###############################################################################


if( DAKOTADRIVER )
  find_package(Dakota REQUIRED)
endif()
find_package(Boost REQUIRED COMPONENTS serialization filesystem system program_options regex)

###########################  Compiler setup ###################################
####################### (post 'project()' command) ############################
set(GCC_ADDITIONAL_WARNINGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wsuggest-override -pedantic")

message("Trilinos_CXX_COMPILER_FLAGS: ${Trilinos_CXX_COMPILER_FLAGS}")
# set(CMAKE_C_FLAGS "${Trilinos_C_COMPILER_FLAGS} -fPIC -fopenmp")
# set(CMAKE_CXX_FLAGS "${Trilinos_CXX_COMPILER_FLAGS} -fPIC -fopenmp")
set(CMAKE_C_FLAGS "${Trilinos_C_COMPILER_FLAGS} -fPIC -Werror")
set(CMAKE_CXX_FLAGS "${Trilinos_CXX_COMPILER_FLAGS} -fPIC -Werror ${GCC_ADDITIONAL_WARNINGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")


############################## gtest Setup ####################################
###############################################################################
if( UNIT_TESTING )
  find_package(GTest REQUIRED)
endif()

if( ESP_ENABLED )
  Plato_find_lib ( ESP_LIB ON egads ${ESP_LIB_DIR} )
  Plato_find_lib ( OCSM_LIB ON ocsm ${ESP_LIB_DIR} )
  Plato_find_path( ESP_INC ON egads.h ${ESP_INC_DIR} )
  include_directories( ${ESP_INC} )
endif()

###############################################################################
###############################################################################
if("${CMAKE_BUILD_TYPE}" STREQUAL Release)
  add_definitions( -DNDEBUG)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
endif()

if( STK_ENABLED )
  message( "-- Compiling with STK " )
  add_definitions( -DSTK_ENABLED )
endif()

if( ESP_ENABLED )
  message( "-- Compiling with ESP " )
  add_definitions( -DESP_ENABLED )
endif()

if( DAKOTADRIVER )
  message( "-- Compiling Dakota Driver " )
  add_definitions(-DDAKOTADRIVER)
endif()

if( ENABLE_ISO )
  message( "-- Compiling PlatoIso " )
  add_definitions(-DENABLE_ISO)
endif()

if( ENABLE_PRUNE )
  message( "-- Compiling Plato prune and refine " )
  add_definitions(-DENABLE_PRUNE)
endif()



if( REGRESSION OR UNIT_TESTING )
    enable_testing()
endif()

if( AMFILTER_ENABLED )
    message( "-- Compiling AMFilter, c++14 required " )
    string(REPLACE "-std=c++11" "-std=c++14" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "-std=c++11" "-std=c++14" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "-std=c++11" "-std=c++14" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    add_definitions(-DAMFILTER_ENABLED)
    find_package(ArborX REQUIRED)
    set(PLATO_LIBRARIES ${PLATO_LIBRARIES} ArborX::ArborX)
endif()

add_subdirectory(base)
add_subdirectory(apps)

include(CMakePackageConfigHelpers)
install(EXPORT PlatoEngine NAMESPACE PlatoEngine FILE PlatoEngineTargets.cmake DESTINATION cmake)
configure_package_config_file(PlatoEngineConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PlatoEngineConfig.cmake INSTALL_DESTINATION cmake)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PlatoEngineConfig.cmake DESTINATION cmake)


install(EXPORT PlatoEngine NAMESPACE PlatoEngine:: FILE PlatoEngineTargets.cmake DESTINATION cmake)

if( REGRESSION )
  integration_tests_dir()
  if( PLATOMAIN )
    add_integration_test(support_structure)
    add_integration_test(su2_to_exodus)
    if( PLATOPROXY )
      add_integration_test(proxy)
    endif()
    if( PLATOSTATICS )
      add_integration_test(platostatics)
    endif()
    if( ENABLE_PRUNE )
        add_integration_test(prune_and_refine)
    endif()
    if( SIERRA_TESTS_ENABLED )
      install_prebuilt_binaries()
      add_integration_test(sierra)
    endif()
    if( SIERRA_TESTS_ENABLED AND DAKOTADRIVER )
      add_integration_test(dakota_sierra)
    endif()
    if( OPTIMISM_TESTS_ENABLED )
      add_integration_test(optimism)
    endif()
  else()
    message("-- !! -- PlatoMain is required for regression testing.")
    message("-- !! -- Regression has not been enabled.")
  endif()
endif()

add_subdirectory(share)
