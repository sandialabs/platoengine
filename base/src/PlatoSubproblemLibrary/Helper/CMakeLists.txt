set(SOURCES 
    PSL_FreeHelpers.cpp
    PSL_FreeStandardization.cpp
    PSL_GradientCheck.cpp
    PSL_LinearCongruentialGenerator.cpp
    PSL_Random.cpp
    )

set(HEADERS 
    PSL_Constants.hpp
    PSL_FreeHelpers.hpp
    PSL_FreeStandardization.hpp
    PSL_GradientCheck.hpp
    PSL_LinearCongruentialGenerator.hpp
    PSL_Random.hpp
    PSL_UnitTestingHelper.hpp
    )

add_library(PlatoPSLHelper ${SOURCES} ${HEADERS})
target_include_directories(PlatoPSLHelper PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLHelper PUBLIC PlatoPSLGeometry)

install( TARGETS PlatoPSLHelper EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLHelper PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

