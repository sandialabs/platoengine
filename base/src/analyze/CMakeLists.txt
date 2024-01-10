
SET(SOURCES data_mesh.cpp
            data_container.cpp
            mesh_io.cpp
            mesh_services.cpp
            communicator.cpp
            topological_element.cpp
            exception_handling.cpp
            exodus_io.cpp
            matrix_container.cpp
            material_container.cpp
            math_library.cpp
            linear_elastic.cpp
            lightmp.cpp
            nemesis_io.cpp
            solid_statics.cpp)

SET(HEADERS data_mesh.hpp
            types.hpp
            data_container.hpp
            mesh_io.hpp
            mesh_services.hpp
            communicator.hpp
            topological_element.hpp
            exception_handling.hpp
            matrix_container.hpp
            material_container.hpp
            math_library.hpp
            linear_elastic.hpp
            lightmp.hpp
            solid_statics.hpp
            bc.hpp)



add_library(PlatoAnalyze ${SOURCES} ${HEADERS})
target_link_libraries(PlatoAnalyze PUBLIC PlatoOperations PlatoParser Intrepid::intrepid PRIVATE PlatoPenaltyModels ML::ml Epetra::epetra AztecOO::aztecoo SEACASNemesis::nemesis SEACASExodus::exodus)
target_include_directories(PlatoAnalyze PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoAnalyze EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         )
target_include_directories(PlatoAnalyze PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
