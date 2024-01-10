
set( SOURCES
  Plato_DakotaAppInterface.cpp
  Plato_DakotaDataMap.cpp
  Plato_DakotaAppInterfaceUtilities.cpp
  )
set( HEADERS
  Plato_DakotaAppInterface.hpp
  Plato_DakotaDataMap.hpp
  Plato_DakotaDriver.hpp
  Plato_DakotaAppInterfaceUtilities.hpp
  )
                        

add_library( PlatoDakotaDriver ${SOURCES} ${HEADERS} )
target_link_libraries(PlatoDakotaDriver PRIVATE PlatoParser PlatoInterface ${Dakota_LIBRARIES})
target_include_directories(PlatoDakotaDriver PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(PlatoDakotaDriver SYSTEM PUBLIC $<BUILD_INTERFACE:${Dakota_INCLUDE_DIRS}>)
target_compile_definitions(PlatoDakotaDriver PRIVATE ${Dakota_DEFINES})

install( TARGETS PlatoDakotaDriver EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         )
install( FILES ${HEADERS} DESTINATION include)

