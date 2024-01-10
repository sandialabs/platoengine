set(SOURCES 
    PSL_Abstract_BoundedSupportFunction.cpp
    PSL_BoundedSupportFunctionFactory.cpp
    PSL_OverhangInclusionFunction.cpp
    PSL_PolynomialTentFunction.cpp
    )

set(HEADERS 
    PSL_Abstract_BoundedSupportFunction.hpp
    PSL_BoundedSupportFunctionFactory.hpp
    PSL_OverhangInclusionFunction.hpp
    PSL_PolynomialTentFunction.hpp
    )

add_library(PlatoPSLBoundedSupportFunction ${SOURCES} ${HEADERS} )
target_include_directories(PlatoPSLBoundedSupportFunction PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLBoundedSupportFunction PUBLIC PlatoPSLParameterData)

install( TARGETS PlatoPSLBoundedSupportFunction EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLBoundedSupportFunction PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

