
set(SOURCES InterfaceTestFixture.cpp)
set(HEADERS InterfaceTestFixture.hpp)

add_library(PlatoTestUtilities ${SOURCES} ${HEADERS})

target_include_directories(PlatoTestUtilities PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoTestUtilities PUBLIC PlatoOptimize PlatoInterface GTest::GTest)

install( TARGETS PlatoTestUtilities EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
         
