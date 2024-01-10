SET(SRCS UnitMain.cpp
         Plato_Test_ROLGradientCheck.cpp
         Plato_Test_ParseInterfaceOptimizerOptions.cpp)

add_executable(OptimizerInterfaceUnitTester ${SRCS})
target_link_libraries(OptimizerInterfaceUnitTester GTest::GTest PlatoTestUtilities PlatoParser PlatoInterface)

add_test(NAME OptimizerInterfaceUnitTester COMMAND OptimizerInterfaceUnitTester)
set_property(TEST OptimizerInterfaceUnitTester PROPERTY LABELS "small")

install( TARGETS OptimizerInterfaceUnitTester EXPORT PlatoEngine DESTINATION bin )
