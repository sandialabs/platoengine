set(SOURCES PSL_InterfaceToEngine_ParameterDataBuilder.cpp
	PSL_InterfaceToEngine_OptimizationMesh.cpp)
set(HEADERS PSL_InterfaceToEngine_ParameterDataBuilder.hpp
	PSL_InterfaceToEngine_OptimizationMesh.hpp)

add_library(PlatoPlatoKernelLibraryInterface     ${SOURCES}     ${HEADERS}     )
target_link_libraries(PlatoPlatoKernelLibraryInterface PRIVATE PlatoPSLAbstractInterface PlatoParser PlatoAnalyze Boost::serialization)
target_include_directories(PlatoPlatoKernelLibraryInterface PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoPlatoKernelLibraryInterface EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPlatoKernelLibraryInterface PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
