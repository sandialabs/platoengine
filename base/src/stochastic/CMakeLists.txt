set(SOURCES Plato_SampleIndexing.cpp
            Plato_SampleParameterMap.cpp
            Plato_SampleValidation.cpp
            Plato_StochasticSample.cpp)

set(HEADERS Plato_BatchSampleEvaluator.hpp
            Plato_ROLSampleGeneratorResponseCache.hpp
            Plato_SampleIndexing.hpp
            Plato_SampleParameterMap.hpp
            Plato_SampleResponseCache.hpp
            Plato_SampleValidation.hpp
            Plato_StochasticSample.hpp)

add_library(PlatoStochastic ${SOURCES} ${HEADERS})
target_include_directories(PlatoStochastic PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoStochastic PUBLIC PlatoInterface PlatoUtilities PlatoLinearAlgebra PRIVATE PlatoParser)

install( TARGETS PlatoStochastic EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)

target_include_directories(PlatoStochastic PUBLIC $<INSTALL_INTERFACE:include>)
install(FILES ${HEADERS} DESTINATION include)

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
