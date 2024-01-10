set(SOURCES 
    PSL_Abstract_MatrixAssemblyAgent.cpp
    PSL_Abstract_MatrixNormalizationAgent.cpp
    PSL_Abstract_MeshScaleAgent.cpp
    PSL_Abstract_PointGhostingAgent.cpp
    PSL_Abstract_SymmetryPlaneAgent.cpp
    PSL_ByNarrowClone_SymmetryPlaneAgent.cpp
    PSL_ByNarrowShare_PointGhostingAgent.cpp
    PSL_ByOptimizedElementSide_MeshScaleAgent.cpp
    PSL_ByRow_MatrixAssemblyAgent.cpp
    PSL_Default_MatrixNormalizationAgent.cpp
    PSL_RegionOfInterestGhostingAgent.cpp
    )

set(HEADERS 
    PSL_Abstract_MatrixAssemblyAgent.hpp
    PSL_Abstract_MatrixNormalizationAgent.hpp
    PSL_Abstract_MeshScaleAgent.hpp
    PSL_Abstract_PointGhostingAgent.hpp
    PSL_Abstract_SymmetryPlaneAgent.hpp
    PSL_ByNarrowClone_SymmetryPlaneAgent.hpp
    PSL_ByNarrowShare_PointGhostingAgent.hpp
    PSL_ByOptimizedElementSide_MeshScaleAgent.hpp
    PSL_ByRow_MatrixAssemblyAgent.hpp
    PSL_Default_MatrixNormalizationAgent.hpp
    PSL_RegionOfInterestGhostingAgent.hpp
    )

add_library(PlatoPSLAgent ${SOURCES} ${HEADERS} )
target_include_directories(PlatoPSLAgent PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLAgent PUBLIC PlatoPSLParameterData PlatoPSLBoundedSupportFunction)

install( TARGETS PlatoPSLAgent EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLAgent PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

