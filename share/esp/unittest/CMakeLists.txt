file(GLOB ESP_PYTHON_TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.py *.json)
file(COPY ${ESP_PYTHON_TEST_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_SOURCE_DIR}/share/esp/ESPtools.py
               ${CMAKE_BINARY_DIR}/share/esp/unittest/ESPtools.py COPYONLY)

enable_testing()
add_test(NAME PlatoCliUtilitiesTests COMMAND python ${CMAKE_BINARY_DIR}/share/esp/unittest/testPlatoCliUtilities.py)
add_test(NAME MeshMorphTests COMMAND python ${CMAKE_BINARY_DIR}/share/esp/unittest/testMeshMorph.py)
add_test(NAME Aflr2SettingsTests COMMAND python ${CMAKE_BINARY_DIR}/share/esp/unittest/testAflr2Settings.py)
set_tests_properties(PlatoCliUtilitiesTests MeshMorphTests Aflr2SettingsTests PROPERTIES RUN_SERIAL TRUE)
