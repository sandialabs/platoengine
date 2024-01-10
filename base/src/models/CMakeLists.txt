set(SOURCES Plato_PenaltyModel.cpp Plato_SIMP.cpp)
set(HEADERS Plato_PenaltyModel.hpp Plato_SIMP.hpp)

add_library(PlatoPenaltyModels ${SOURCES} ${HEADERS})
target_link_libraries(PlatoPenaltyModels PRIVATE PlatoParser Boost::serialization)
target_include_directories(PlatoPenaltyModels PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoPenaltyModels EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPenaltyModels PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
