set(SOURCES Plato_Interface.cpp
            Plato_Operation.cpp
            Plato_MultiOperation.cpp
            Plato_SingleOperation.cpp
            Plato_OperationFactory.cpp
            Plato_Stage.cpp
            Plato_Performer.cpp)
set(HEADERS Plato_Application.hpp
            Plato_Interface.hpp
            Plato_Operation.hpp
            Plato_MultiOperation.hpp
            Plato_SingleOperation.hpp
            Plato_OperationFactory.hpp
            Plato_Stage.hpp
            Plato_Performer.hpp)

add_library(PlatoInterface ${SOURCES} ${HEADERS})
target_link_libraries(PlatoInterface PUBLIC PlatoDataLayer PlatoParser TeuchosParameterList::teuchosparameterlist PRIVATE PlatoConsole Boost::serialization)
target_include_directories(PlatoInterface PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoInterface EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoInterface PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

