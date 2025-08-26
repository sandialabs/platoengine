set(Parser_SOURCES Plato_Parser.cpp
            Plato_InputData.cpp
            math_parser/tinyexpr.c
            pugixml.cpp
            Plato_FreeFunctions.cpp
            )
            
set(Parser_HEADERS Plato_Parser.hpp
            Plato_Macros.hpp
            Plato_InputData.hpp
            pugixml.hpp
            pugiconfig.hpp
            Plato_FreeFunctions.hpp
            Plato_EnumTable.hpp
            Plato_Utils.hpp
            Plato_SerializationHeaders.hpp
            )

set(Exceptions_SOURCES Plato_Exceptions.cpp)
set(Exceptions_HEADERS Plato_Exceptions.hpp)

set(Utilities_HEADERS Plato_NamedType.hpp)

add_library(PlatoExceptions ${Exceptions_SOURCES} ${Parser_HEADERS}     )
target_link_libraries(PlatoExceptions PRIVATE TeuchosParameterList::teuchosparameterlist Boost::serialization)

add_library(PlatoUtilities INTERFACE ${Utilities_HEADERS})

add_library(PlatoParser ${Parser_SOURCES} ${Parser_HEADERS}     )
target_include_directories(PlatoParser PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoParser PUBLIC   PlatoExceptions PlatoUtilities Boost::serialization)

install( TARGETS PlatoParser PlatoExceptions PlatoUtilities EXPORT PlatoEngine
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)
target_include_directories(PlatoParser PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${LIBLESS_HEADERS} ${Parser_HEADERS} ${Exceptions_HEADERS} ${Console_HEADERS} ${Utilities_HEADERS} DESTINATION include)
install( FILES math_parser/tinyexpr.h DESTINATION include/math_parser)
