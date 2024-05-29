SET(SRCS PlatoKrinoUnitMain.cpp  PlatoKrinoUnitTest.cpp)

add_executable(PlatoKrino_UnitTester ${SRCS})
target_include_directories(PlatoKrino_UnitTester PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
target_link_libraries(PlatoKrino_UnitTester GTest::GTest PlatoParser PlatoKrinoLib)
add_test(NAME PlatoKrino_UnitTester COMMAND PlatoKrino_UnitTester)
set_property(TEST PlatoKrino_UnitTester PROPERTY LABELS "small")

install( TARGETS PlatoKrino_UnitTester EXPORT PlatoEngine DESTINATION bin )
