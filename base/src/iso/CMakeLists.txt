
add_subdirectory(main)

set(sources ExtractMain.cpp)
set(headers main/IsoVolumeExtractionTool.hpp
            main/IVEMeshAPI.hpp
            main/IVEMeshAPISTK.hpp
            main/STKExtract.hpp
            main/IsoVector.hpp)

add_executable(extract_iso ${sources} ${headers})

target_link_libraries(extract_iso PUBLIC IsoExtract IsoExtract)

install( TARGETS extract_iso EXPORT PlatoEngine
         RUNTIME DESTINATION bin)
