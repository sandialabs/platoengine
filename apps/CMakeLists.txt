
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if( ESP_ENABLED )
  add_subdirectory(esp)
endif()

if( PLATOMAIN )
  message( "-- Compiling PlatoMain " )
  add_subdirectory(services)
endif()

if(PYTHON_INTERPRETER_APP)
    message( "-- Compiling python interpreter app " )
    add_subdirectory(python_interpreter)
endif()

if( PLATOPROXY )
  message( "-- Compiling PlatoProxy " )
  add_subdirectory(proxy)
  add_subdirectory(rocket)
  add_subdirectory(ksal_proxy)
  add_subdirectory(rosenbrock)
endif()

if( PLATOSTATICS )
  message( "-- Compiling Statics " )
  add_subdirectory(statics)
endif()

if(EXPY)
  message( "-- Compiling Expy " )
  add_subdirectory(services/python)
endif()
