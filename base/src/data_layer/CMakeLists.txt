set(SOURCES Plato_DataLayer.cpp
            Plato_SharedField.cpp
            Plato_SharedValue.cpp
            Plato_SharedDataInfo.cpp)
set(HEADERS Plato_DataLayer.hpp
            Plato_SharedData.hpp
            Plato_SharedField.hpp
            Plato_SharedValue.hpp
            Plato_SharedDataInfo.hpp)

add_library(PlatoDataLayer ${SOURCES} ${HEADERS})
target_include_directories(PlatoDataLayer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoDataLayer PRIVATE PlatoInterface Epetra::epetra)

install( TARGETS PlatoDataLayer EXPORT PlatoEngine
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )
target_include_directories(PlatoDataLayer PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
