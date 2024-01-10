SET(SRCS Plato_Test_DefinedEnums.cpp
         Plato_Test_FileList.cpp
         Plato_Test_InputBlockStruct.cpp
         Plato_Test_InputBlockWithFileTypes.cpp
         Plato_Test_ParseEnum.cpp
         Plato_Test_Parser.cpp
         UnitMain.cpp )

set(PlatoInputParser_UnitTester_LIBS GTest::GTest PlatoInputParser)

add_executable(PlatoInputParser_UnitTester ${SRCS})
target_link_libraries(PlatoInputParser_UnitTester PRIVATE ${PlatoInputParser_UnitTester_LIBS})

add_test(NAME PlatoInputParser_UnitTester COMMAND PlatoInputParser_UnitTester)
set_property(TEST PlatoInputParser_UnitTester PROPERTY LABELS "small")

install( TARGETS PlatoInputParser_UnitTester DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
