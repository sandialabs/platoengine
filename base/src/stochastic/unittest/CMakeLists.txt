SET(SRCS Plato_Test_ROLSampleGeneratorResponseCache.cpp
         Plato_Test_SampleIndexing.cpp
         Plato_Test_SampleParameterMap.cpp
         Plato_Test_SampleResponseCache.cpp
         Plato_Test_SampleValidation.cpp
         UnitMain.cpp )

set(LIBS PlatoStochastic
         GTest::GTest 
         ROL::rol )
 
add_executable(Plato_Stochastic_UnitTester ${SRCS})
target_link_libraries(Plato_Stochastic_UnitTester PRIVATE ${LIBS})

add_test(NAME Plato_Stochastic_UnitTester COMMAND Plato_Stochastic_UnitTester)
set_property(TEST Plato_Stochastic_UnitTester PROPERTY LABELS "small")

install(TARGETS Plato_Stochastic_UnitTester DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
