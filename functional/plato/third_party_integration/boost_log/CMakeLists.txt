include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Boost::log
              PlatoFunctionalComponents
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalBoostLog "." "${LINK_LIBS}")

if(UNIT_TESTING)
    create_plato_library(PlatoFunctionalBoostLogTestUtilities "test_utilities" "Boost::log")

    create_plato_unittester(PlatoFunctionalBoostLog unittest "PlatoFunctionalBoostLogTestUtilities")

    set(NUM_RANKS_FOR_TEST 2)
    create_plato_parallel_unittester(PlatoFunctionalBoostLog parallel_unittest ${NUM_RANKS_FOR_TEST} "PlatoFunctionalBoostLogTestUtilities")
endif()
