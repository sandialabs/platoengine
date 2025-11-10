include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

set(LINK_LIBS GTest::GTest
              Kokkos::kokkos
              Boost::headers
              Boost::mpi
              Boost::log
              MPI::MPI_C)

create_plato_library(PlatoFunctionalTestUtilities "." "${LINK_LIBS}")

set(INTERFACE_FILES TestContext.hpp GradientChecker.hpp GradientCheckerImpl.hpp)
create_plato_interface(PlatoTestUtilitiesInterface "${INTERFACE_FILES}" "")

if(UNIT_TESTING)
    create_plato_unittester(PlatoFunctionalTestUtilities "unittest" )
    set(NUM_RANKS_FOR_TEST 4)
    create_plato_parallel_unittester(PlatoFunctionalTestUtilities "parallel_unittest" ${NUM_RANKS_FOR_TEST})
endif()
