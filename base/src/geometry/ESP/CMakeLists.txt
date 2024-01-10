set(LIB_NAMES)

set(LIB_NAME GeometryESP)
set(LIB_NAMES ${LIB_NAMES} ${LIB_NAME})

set(ESP_SOURCES
       Plato_ESP.cpp
   )

set(ESP_HEADERS
       Plato_ESP.hpp
   )

add_library(PlatoGeometryESP ${ESP_SOURCES} ${ESP_HEADERS})
target_include_directories(PlatoGeometryESP PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoGeometryESP PRIVATE PlatoOptimize)

set(ESPI_SOURCES
       Plato_ESP_Impl.cpp
       bodyTess.c
   )

set(ESPI_HEADERS
       bodyTess.h
       Plato_ESP.hpp
   )

add_library(PlatoGeometryESPImpl SHARED ${ESPI_SOURCES} ${ESPI_HEADERS})
target_include_directories(PlatoGeometryESPImpl PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoGeometryESPImpl PRIVATE PlatoGeometryESP PlatoOptimize PlatoExceptions ${OCSM_LIB} ${ESP_LIB})

install( TARGETS PlatoGeometryESP PlatoGeometryESPImpl EXPORT PlatoEngine
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib
           )
target_include_directories(PlatoGeometryESP PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories(PlatoGeometryESPImpl PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${ESP_HEADERS} ${ESPI_HEADERS} DESTINATION include)

#if( UNIT_TESTING )
#  add_subdirectory(./unittest)
#endif()
