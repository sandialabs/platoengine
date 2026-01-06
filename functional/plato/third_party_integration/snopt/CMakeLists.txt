include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS ${SNOPT_LIB_DIR}/libsnopt7_cpp.so
              PlatoFunctionalCore
              PlatoFunctionalUtilities
              PlatoFunctionalLinearAlgebra
              PlatoFunctionalOutput)

create_plato_library(PlatoFunctionalSNOPT "." "${LINK_LIBS}")
target_include_directories(PlatoFunctionalSNOPT PRIVATE ${SNOPT_INC_DIR})

if(UNIT_TESTING)
    create_plato_unittester(PlatoFunctionalSNOPT "unittest;test_utilities")
    target_include_directories(PlatoFunctionalSNOPT_UnitTester PRIVATE ${SNOPT_INC_DIR})
endif()
