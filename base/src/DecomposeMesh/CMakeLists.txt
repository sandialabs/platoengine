
set(SOURCES Plato_DecomposeMesh.cpp)
set(HEADERS Plato_DecomposeMesh.hpp)

add_library(PlatoDecomposeMesh ${SOURCES} ${HEADERS})
target_include_directories(PlatoDecomposeMesh PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoDecomposeMesh PRIVATE STKIO::stk_io)

install( TARGETS PlatoDecomposeMesh EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoDecomposeMesh PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
