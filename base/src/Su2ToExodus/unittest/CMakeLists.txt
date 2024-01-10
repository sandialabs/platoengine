SET(SRCS Su2ToExodus_UnitMain.cpp
         Plato_Test_Su2ToExodus.cpp
         Su2ToExodus_UnitTester.cpp
         ../Su2ToExodus.cpp)

add_executable(PlatoSu2ToExodus_UnitTester ${SRCS})
target_link_libraries(PlatoSu2ToExodus_UnitTester SEACASExodus::exodus GTest::GTest)
target_include_directories(PlatoSu2ToExodus_UnitTester PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)

add_test(NAME PlatoSu2ToExodus_UnitTester COMMAND PlatoSu2ToExodus_UnitTester)
set_property(TEST PlatoSu2ToExodus_UnitTester PROPERTY LABELS "small")

install( TARGETS PlatoSu2ToExodus_UnitTester EXPORT PlatoEngine DESTINATION bin )
