set(SOURCES 
    PSL_Filter.cpp
    PSL_KernelFilter.cpp
    PSL_KernelThenHeavisideFilter.cpp
    PSL_KernelThenTANHFilter.cpp
    PSL_ProjectionHeavisideFilter.cpp
    PSL_ProjectionTANHFilter.cpp
    PSL_AbstractKernelThenFilter.cpp
    PSL_AbstractProjectionFilter.cpp
    PSL_AbstractKernelThenProjection.cpp
    PSL_OverhangFilter.cpp
    )

set(HEADERS 
    PSL_Filter.hpp
    PSL_KernelFilter.hpp
    PSL_KernelThenHeavisideFilter.hpp
    PSL_KernelThenTANHFilter.hpp
    PSL_ProjectionHeavisideFilter.hpp
    PSL_ProjectionTANHFilter.hpp
    PSL_AbstractKernelThenFilter.hpp
    PSL_AbstractProjectionFilter.hpp
    PSL_AbstractKernelThenProjection.hpp
    PSL_OverhangFilter.hpp
    )

if( AMFILTER_ENABLED )
  list(APPEND SOURCES PSL_KernelThenStructuredAMFilter.cpp PSL_AMFilterUtilities.cpp)
  list(APPEND HEADERS PSL_KernelThenStructuredAMFilter.hpp PSL_AMFilterUtilities.hpp)
endif()

add_library(PlatoPSLFilter ${SOURCES} ${HEADERS})
target_link_libraries(PlatoPSLFilter PUBLIC PlatoPSLAbstractInterface PlatoPSLAgent)
target_include_directories(PlatoPSLFilter PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoPSLFilter EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLFilter PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

