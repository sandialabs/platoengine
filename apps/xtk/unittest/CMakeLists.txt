###############################################################################
# Sources:
###############################################################################
SET(SRCS UnitMainXTK.cpp
         Plato_Test_XTK_Application.cpp)

SET(HDRS )

SET( testFiles "xtkApp.xml;xtkInput.xml;ut_xtk_bolted_bracket.exo" )
Plato_add_test_files("${testFiles}")

add_executable(PlatoXTK_UnitTester ${SRCS})
target_link_libraries(PlatoXTK_UnitTester PlatoXTK GTest::GTest)
add_test(NAME PlatoXTK_UnitTester COMMAND PlatoXTK_UnitTester)

install( TARGETS PlatoXTK_UnitTester DESTINATION bin )
