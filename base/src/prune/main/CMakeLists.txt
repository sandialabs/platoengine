set(SOURCES MeshBalance.cpp
            MeshManager.cpp
            MeshPrune.cpp
            MeshTransfer.cpp
            PerceptPrune.cpp
            PerceptPruneTool.cpp
            PruneMeshAPISTK.cpp
            PruneTool.cpp
            STKPrune.cpp
            MeshRefine.cpp)
set(HEADERS MeshBalance.hpp
            MeshManager.hpp
            MeshPrune.hpp
            MeshTransfer.hpp
            PerceptPrune.hpp
            PerceptPruneTool.hpp
            PruneMeshAPISTK.hpp
            PruneMeshAPI.hpp
            PruneTool.hpp
            STKPrune.hpp
            MeshRefine.hpp)

add_library(Prune ${SOURCES} ${HEADERS})

target_include_directories(Prune PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(Prune PUBLIC Percept::percept)

install( TARGETS Prune EXPORT Prune
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
