
SET(SRCS StructuralTopologyOptimizationProxyMain.cpp)
SET(HDRS )

add_executable(TopoProxy ${SRCS})
target_link_libraries(TopoProxy PUBLIC PlatoProxy PlatoInterface)

install( TARGETS TopoProxy EXPORT PlatoEngine DESTINATION bin )
