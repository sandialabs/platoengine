set(SRCS 
  UnitMain.cpp
  Plato_Test_SIMP.cpp)

add_executable(PlatoModels_UnitTester ${SRCS})
target_link_libraries(PlatoModels_UnitTester PRIVATE GTest::GTest PlatoPenaltyModels PlatoParser PlatoExceptions)
add_test(NAME PlatoModels_UnitTester COMMAND PlatoModels_UnitTester)
set_property(TEST PlatoModels_UnitTester PROPERTY LABELS "small")

install( TARGETS PlatoModels_UnitTester EXPORT PlatoEngine DESTINATION bin )
