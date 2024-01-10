set(SOURCES Plato_StructuralTopologyOptimization.cpp
            Plato_StructuralTopologyOptimizationProxyApp.cpp
            )

set(HEADERS Plato_ProxyVolume.hpp
						Plato_Cylinder.hpp
						Plato_GeometryModel.hpp
						Plato_ProxyCompliance.hpp
						Plato_AlgebraicRocketModel.hpp
						Plato_GradBasedRocketObjective.hpp
						Plato_StructuralTopologyOptimization.hpp
						Plato_StructuralTopologyOptimizationProxyApp.hpp
						)

add_library(PlatoProxy ${SOURCES} ${HEADERS})
target_include_directories(PlatoProxy PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoProxy PRIVATE PlatoInterface PlatoDataLayer Epetra::epetra)

install( TARGETS PlatoProxy EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)

target_include_directories(PlatoProxy PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
