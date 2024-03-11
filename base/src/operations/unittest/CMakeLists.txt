SET(SRCS ../Plato_InitializeValues.cpp
         Plato_Test_Operations.cpp
         Plato_Test_Operations_Boost.cpp
         Plato_Test_OperationUtilities.cpp
         Operations_UnitMain.cpp
         )

add_executable(Plato_Operations_UnitTester ${SRCS})
target_link_libraries(Plato_Operations_UnitTester PlatoApp GTest::GTest Boost::serialization )

add_test(NAME Plato_Operations_UnitTester COMMAND Plato_Operations_UnitTester)
set_property(TEST Plato_Operations_UnitTester PROPERTY LABELS "small")

install( TARGETS Plato_Operations_UnitTester EXPORT PlatoEngine DESTINATION bin )
