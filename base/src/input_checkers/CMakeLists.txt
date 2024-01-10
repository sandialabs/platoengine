SET(InputXMLChecker_SRCS InputXMLChecker_Main.cpp InputXMLChecker.cpp)
SET(InputXMLChecker_HDRS InputXMLChecker.hpp)

add_executable(InputXMLChecker ${InputXMLChecker_SRCS})
target_link_libraries(InputXMLChecker PUBLIC PlatoParser)

install( TARGETS InputXMLChecker EXPORT PlatoEngine DESTINATION bin )

