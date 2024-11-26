SET(SRCS PlatoPython_main.cpp 
         PlatoPythonApp.cpp
         PythonAppUtilities.cpp
         PythonOperationFactory.cpp
         PlatoPythonOperation.cpp
         CriterionValueOperation.cpp
         CriterionGradientOperation.cpp
         RunVoidFunctionOperation.cpp)

SET(HDRS PlatoPythonApp.hpp
         PythonAppUtilities.hpp
         PythonOperationFactory.hpp
         PlatoPythonOperation.hpp
         CriterionValueOperation.hpp
         CriterionGradientOperation.hpp
         CriterionValueAndGradientOperation.hpp
         RunVoidFunctionOperation.hpp)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

add_executable(PythonInterpreter ${SRCS})
target_link_libraries(PythonInterpreter  
                      PUBLIC PlatoInterface PlatoParser PlatoExceptions
                      PRIVATE pybind11::embed)

install( TARGETS PythonInterpreter EXPORT PlatoEngine DESTINATION bin )

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
