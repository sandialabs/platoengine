set(SOURCES 
    PSL_AxisAlignedBoundingBox.cpp
    PSL_PointCloud.cpp
    PSL_Point.cpp
    PSL_Vector.cpp
    PSL_Triangle.cpp
    PSL_OrthogonalGridUtilities.cpp
    PSL_RegularHex8.cpp
    )

set(HEADERS 
    PSL_AxisAlignedBoundingBox.hpp
    PSL_PointCloud.hpp
    PSL_Point.hpp
    PSL_Vector.hpp
    PSL_Triangle.hpp
    PSL_OrthogonalGridUtilities.hpp
    PSL_RegularHex8.hpp
    )

if( AMFILTER_ENABLED )
  list(APPEND SOURCES PSL_TetMeshUtilities.cpp)
  list(APPEND HEADERS PSL_TetMeshUtilities.hpp)
endif()

add_library(PlatoPSLGeometry ${SOURCES} ${HEADERS})
target_include_directories(PlatoPSLGeometry PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLGeometry PUBLIC PlatoPSLHelper)

install( TARGETS PlatoPSLGeometry EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLGeometry PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

