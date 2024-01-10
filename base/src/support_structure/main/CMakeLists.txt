set(SOURCES MeshWrapper.cpp
            SupportStructure.cpp)
set(HEADERS MeshWrapper.hpp
            SupportStructure.hpp)

add_library(SupportStructureLib ${SOURCES} ${HEADERS})
target_include_directories(SupportStructureLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(SupportStructureLib PUBLIC STKIO::stk_io STKMesh::stk_mesh_base SEACASIoss::Ioss)

install( TARGETS SupportStructureLib EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
