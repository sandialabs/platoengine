SET(SRCS PlatoKrinoApp.cpp 
         PlatoKrinoParse.cpp
         PlatoKrinoUtilities.cpp
         PlatoKrinoMain.cpp
         PlatoKrinoInterface.cpp)

SET(HDRS PlatoKrinoEnums.hpp
         PlatoKrinoInterface.hpp
         PlatoKrinoApp.hpp
         PlatoKrinoParse.hpp
         PlatoKrinoUtilities.hpp)

add_executable(PlatoKrino ${SRCS})
target_link_libraries(PlatoKrino PUBLIC PlatoInterface Krino::krino_lib)

install( TARGETS PlatoKrino EXPORT PlatoEngine DESTINATION bin )

