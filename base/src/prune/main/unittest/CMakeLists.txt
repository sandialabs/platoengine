SET(SRCS 
        PruneAndRefineTest.cpp 
	UnitMain.cpp )

add_executable(Plato_PruneAndRefine_UnitTester ${SRCS})
target_link_libraries(Plato_PruneAndRefine_UnitTester GTest::GTest Prune Percept::percept)

add_test(NAME Plato_PruneAndRefine_UnitTester COMMAND Plato_PruneAndRefine_UnitTester)
set_property(TEST Plato_PruneAndRefine_UnitTester PROPERTY LABELS "small")

install( TARGETS Plato_PruneAndRefine_UnitTester EXPORT PlatoEngine DESTINATION bin )
