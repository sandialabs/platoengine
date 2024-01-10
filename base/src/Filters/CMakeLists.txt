set(SOURCES
    PlatoEngine_KernelFilter.cpp
    PlatoEngine_KernelThenHeavisideFilter.cpp
    PlatoEngine_KernelThenTANHFilter.cpp
    PlatoEngine_ProjectionHeavisideFilter.cpp
    PlatoEngine_ProjectionTANHFilter.cpp
    PlatoEngine_AbstractKernelThenFilter.cpp
    PlatoEngine_AbstractProjectionFilter.cpp
    PlatoEngine_AbstractFilter.cpp
    PlatoEngine_FilterFactory.cpp
    PlatoEngine_IdentityFilter.cpp
    )
set(HEADERS
    PlatoEngine_KernelFilter.hpp
    PlatoEngine_KernelThenHeavisideFilter.hpp
    PlatoEngine_KernelThenTANHFilter.hpp
    PlatoEngine_ProjectionHeavisideFilter.hpp
    PlatoEngine_ProjectionTANHFilter.hpp
    PlatoEngine_AbstractKernelThenFilter.hpp
    PlatoEngine_AbstractProjectionFilter.hpp
    PlatoEngine_AbstractFilter.hpp
    PlatoEngine_FilterFactory.hpp
    PlatoEngine_IdentityFilter.hpp
    )

if( AMFILTER_ENABLED )
  list(APPEND SOURCES PlatoEngine_KernelThenStructuredAMFilter.cpp)
  list(APPEND HEADERS PlatoEngine_KernelThenStructuredAMFilter.hpp)
endif()


add_library(PlatoFilters ${SOURCES} ${HEADERS} )
target_include_directories(PlatoFilters PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoFilters PUBLIC PlatoParser PRIVATE PlatoInterface PlatoAnalyze PlatoPSLFilter PlatoPlatoKernelLibraryInterface Boost::serialization)

install( TARGETS PlatoFilters EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoFilters PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
