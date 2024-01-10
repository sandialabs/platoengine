
set(SOURCES IsoVolumeExtractionTool.cpp
                        IVEMeshAPISTK.cpp
                        STKExtract.cpp
                        IsoVector.cpp)
set(HEADERS IsoVolumeExtractionTool.hpp
                       IVEMeshAPI.hpp
                       IVEMeshAPISTK.hpp
                       STKExtract.hpp
                       IsoVector.hpp)

add_library(IsoExtract ${SOURCES} ${HEADERS})
target_include_directories(IsoExtract PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(IsoExtract PRIVATE PlatoParser PUBLIC STKIO::stk_io STKMesh::stk_mesh_base SEACASIoss::Ioss)

install( TARGETS IsoExtract EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
         
