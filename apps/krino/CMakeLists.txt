SET(EXE_SRCS PlatoKrinoMain.cpp
             PlatoKrinoApp.cpp
             PlatoKrinoAppUtils.cpp
             PlatoKrinoWrapper.cpp
             PlatoKrinoUtilities.cpp
             PlatoKrinoSphereBuilder.cpp)

# EXE
add_executable(PlatoKrino ${EXE_SRCS})
target_link_libraries(PlatoKrino PUBLIC Krino::krino_lib PlatoInterface PlatoFunctionalUtilities)
install( TARGETS PlatoKrino EXPORT PlatoEngine DESTINATION bin )


