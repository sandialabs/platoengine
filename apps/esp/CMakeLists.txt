SET(SRCS PlatoESP_main.cpp PlatoESPApp.cpp)

SET(HDRS PlatoESPApp.hpp)

add_executable(PlatoESP ${SRCS})
target_link_libraries(PlatoESP PUBLIC PlatoInterface PlatoGeometryESP PlatoOperations)

install( TARGETS PlatoESP EXPORT PlatoEngine DESTINATION bin )

if (PLATO_ENABLE_SERVICES_PYTHON)
    find_package(Python3 COMPONENTS Development Interpreter)
    add_library( PythonPlatoESP SHARED PlatoESPApp.cpp PlatoESPPythonModule.cpp )
    target_include_directories(PythonPlatoESP SYSTEM PRIVATE ${Python3_INCLUDE_DIRS} )
    target_link_libraries( PythonPlatoESP PlatoInterface PlatoGeometryESP PlatoOperations ${Python3_LIBRARIES} )
    set_target_properties( PythonPlatoESP PROPERTIES PREFIX "" )

    install( TARGETS PythonPlatoESP EXPORT PlatoEngine DESTINATION lib )

endif()
