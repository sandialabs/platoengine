SET(SRCS UnitMain.cpp
         ../PlatoPythonApp.cpp
         ../PythonAppUtilities.cpp
         ../PythonOperationFactory.cpp
         ../PlatoPythonOperation.cpp
         ../CriterionValueOperation.cpp
         ../CriterionGradientOperation.cpp
         ../RunVoidFunctionOperation.cpp
	       Plato_Test_PythonApp.cpp)

SET(HDRS )

find_package(Boost REQUIRED COMPONENTS python)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

add_executable(PythonAppUnitTester ${SRCS})
target_include_directories(PythonAppUnitTester PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
target_link_libraries(PythonAppUnitTester 
                      GTest::GTest
                      PlatoInterface PlatoParser PlatoDataLayer PlatoExceptions
                      Python::Python
                      Boost::python)

add_test(NAME PythonAppUnitTester COMMAND PythonAppUnitTester)
set_property(TEST PythonAppUnitTester PROPERTY LABELS "small")

configure_file(  ${CMAKE_SOURCE_DIR}/apps/python_interpreter/unittest/test_functions.py
                 ${CMAKE_BINARY_DIR}/apps/python_interpreter/unittest/test_functions.py COPYONLY)

configure_file(  ${CMAKE_SOURCE_DIR}/apps/python_interpreter/unittest/import_functions.py
                 ${CMAKE_BINARY_DIR}/apps/python_interpreter/unittest/import/import_functions.py COPYONLY)

configure_file(  ${CMAKE_SOURCE_DIR}/apps/python_interpreter/unittest/numpy_functions.py
                 ${CMAKE_BINARY_DIR}/apps/python_interpreter/unittest/import/numpy_functions.py COPYONLY)

install( TARGETS PythonAppUnitTester DESTINATION bin )
file( GLOB PYTHON_FILES *.py )
install( FILES ${PYTHON_FILES} DESTINATION lib )
