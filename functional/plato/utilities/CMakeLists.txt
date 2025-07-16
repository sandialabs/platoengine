include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

set(LINK_LIBS Boost::headers Boost::mpi Boost::filesystem)

configure_file(${FUNCTIONAL_BASE_DIR}/plato/cmake/DataFilePath.cpp ${CMAKE_CURRENT_BINARY_DIR}/configured_src/DataFilePath.cpp)
create_plato_library(PlatoFunctionalUtilities ".;${CMAKE_CURRENT_BINARY_DIR}/configured_src"  "${LINK_LIBS}")

set(INTERFACE_FILES StateCache.hpp
                    NamedType.hpp
                    Zip.hpp
                    ZipIterator.hpp
                    FixedWidthFloatingPointOutput.hpp)
create_plato_interface(PlatoUtilitiesInterface "${INTERFACE_FILES}" "")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalUtilities unittest)
  
  set(NUM_RANKS_FOR_TEST 3)
  create_plato_parallel_unittester(PlatoFunctionalUtilities "parallel_unittest" ${NUM_RANKS_FOR_TEST} "")

endif()
