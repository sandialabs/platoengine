SET(SRCS UnitMain.cpp
         ../Plato_ParseCSMUtilities.cpp
         Plato_Test_InputData.cpp
         Plato_Test_Utilities.cpp
         Plato_Test_EnumTable.cpp
         Plato_Test_EngineDataParser.cpp)

add_executable(PlatoTools_UnitTester ${SRCS})
target_link_libraries(PlatoTools_UnitTester GTest::GTest PlatoParser PlatoExceptions )
add_test(NAME PlatoTools_UnitTester COMMAND PlatoTools_UnitTester)
set_property(TEST PlatoTools_UnitTester PROPERTY LABELS "small")

install( TARGETS PlatoTools_UnitTester EXPORT PlatoEngine DESTINATION bin )
