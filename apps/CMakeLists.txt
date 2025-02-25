include( ${CMAKE_SOURCE_DIR}/base/config/require_feature.cmake )
require_feature( LEGACY_BUILD )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if( ESP_ENABLED )
  add_subdirectory(esp)
endif()

if( PLATOMAIN )
  message( "-- Compiling PlatoMain " )
  add_subdirectory(services)
endif()

if( PLATOPROXY )
  message( "-- Compiling PlatoProxy " )
  add_subdirectory(proxy)
  add_subdirectory(rocket)
  add_subdirectory(ksal_proxy)
  add_subdirectory(rosenbrock)
endif()

if(EXPY)
  message( "-- Compiling Expy " )
  add_subdirectory(services/python)
endif()

if(STK_ENABLED)
  add_subdirectory(krino)
endif()
