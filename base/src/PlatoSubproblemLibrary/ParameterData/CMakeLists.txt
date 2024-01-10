set(SOURCES 
    PSL_Abstract_ParameterData.cpp
    PSL_ParameterData.cpp
    )

set(HEADERS 
    PSL_Abstract_ParameterData.hpp
    PSL_ParameterDataEnums.hpp
    PSL_ParameterData.hpp
    )

add_library(PlatoPSLParameterData ${SOURCES} ${HEADERS})
target_include_directories(PlatoPSLParameterData PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLParameterData PUBLIC PlatoPSLAbstractInterface)

install( TARGETS PlatoPSLParameterData EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLParameterData PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

