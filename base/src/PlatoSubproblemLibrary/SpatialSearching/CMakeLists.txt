set(SOURCES 
    PSL_BoundingBoxBruteForce.cpp
    PSL_BoundingBoxMortonHierarchy.cpp
    PSL_BruteForceFixedRadiusNearestNeighbors.cpp
    PSL_BruteForceNearestNeighbor.cpp
    PSL_RadixGridFixedRadiusNearestNeighbors.cpp
    PSL_SpatialSearcherFactory.cpp
    )

set(HEADERS 
    PSL_BoundingBoxBruteForce.hpp
    PSL_BoundingBoxMortonHierarchy.hpp
    PSL_BruteForceFixedRadiusNearestNeighbors.hpp
    PSL_BruteForceNearestNeighbor.hpp
    PSL_RadixGridFixedRadiusNearestNeighbors.hpp
    PSL_SpatialSearcherFactory.hpp
    )

add_library(PlatoPSLSpatialSearching ${SOURCES} ${HEADERS})
target_include_directories(PlatoPSLSpatialSearching PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLSpatialSearching PUBLIC PlatoPSLAbstractInterface PlatoPSLParameterData)
  
install( TARGETS PlatoPSLSpatialSearching EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
         
target_include_directories(PlatoPSLSpatialSearching PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

