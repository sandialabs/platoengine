configure_file(${CMAKE_SOURCE_DIR}/share/esp/unittest/testPlatoCliUtilities.py
               ${CMAKE_BINARY_DIR}/share/esp/unittest/testPlatoCliUtilities.py COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/share/esp/unittest/testMeshMorph.py
               ${CMAKE_BINARY_DIR}/share/esp/unittest/testMeshMorph.py COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/share/esp/unittest/csmFileGenerator.py
               ${CMAKE_BINARY_DIR}/share/esp/unittest/csmFileGenerator.py COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/share/esp/ESPtools.py
               ${CMAKE_BINARY_DIR}/share/esp/unittest/ESPtools.py COPYONLY)

enable_testing()
add_test(NAME PlatoCliUtilitiesTests COMMAND python ${CMAKE_BINARY_DIR}/share/esp/unittest/testPlatoCliUtilities.py)
add_test(NAME MeshMorphTests COMMAND python ${CMAKE_BINARY_DIR}/share/esp/unittest/testMeshMorph.py)
set_tests_properties(PlatoCliUtilitiesTests MeshMorphTests  PROPERTIES RUN_SERIAL TRUE)
