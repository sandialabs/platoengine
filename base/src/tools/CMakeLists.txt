set(Parser_SOURCES Plato_Parser.cpp
            Plato_InputData.cpp
            Plato_OptimizerEngineStageData.cpp
            Plato_OperationInputDataMng.cpp
            Plato_StageInputDataMng.cpp
            math_parser/tinyexpr.c
            pugixml.cpp
            Plato_UniqueCounter.cpp
            Plato_FreeFunctions.cpp
            Plato_TimersTree.cpp
            Plato_AppErrorChecks.cpp
            Plato_ParseCSMUtilities.cpp
            Plato_OperationTypes.cpp
            )
set(Parser_HEADERS Plato_Parser.hpp
            Plato_Macros.hpp
            Plato_InputData.hpp
            Plato_OptimizerEngineStageData.hpp
            Plato_OperationInputDataMng.hpp
            Plato_StageInputDataMng.hpp
            pugixml.hpp
            pugiconfig.hpp
            Plato_UniqueCounter.hpp
            Plato_FreeFunctions.hpp
            Plato_TimersTree.hpp
            Plato_AppErrorChecks.hpp
            Plato_ParseCSMUtilities.hpp
            Plato_OperationTypes.hpp
            Plato_EnumTable.hpp
            Plato_Utils.hpp
            Plato_SerializationHeaders.hpp
            Plato_SerializationLoadSave.hpp
            Plato_StochasticSampleSharedDataNames.hpp
            )

set(Exceptions_SOURCES Plato_Exceptions.cpp)
set(Exceptions_HEADERS Plato_Exceptions.hpp)

set(Console_SOURCES Plato_Console.cpp)
set(Console_HEADERS Plato_Console.hpp)

set(Utilities_HEADERS Plato_NamedType.hpp)

set(LIBLESS_HEADERS
  Plato_Communication.hpp
)

add_library(PlatoExceptions ${Exceptions_SOURCES} ${Parser_HEADERS}     )
target_link_libraries(PlatoExceptions PRIVATE PlatoInterface Boost::serialization)

add_library(PlatoConsole ${Console_SOURCES} ${Parser_HEADERS}     )
target_link_libraries(PlatoConsole PRIVATE Boost::serialization)

add_library(PlatoUtilities INTERFACE ${Utilities_HEADERS})

add_library(PlatoParser ${Parser_SOURCES} ${Parser_HEADERS}     )
target_include_directories(PlatoParser PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoParser PUBLIC PlatoDataLayer PlatoExceptions PlatoUtilities Boost::serialization)


install( TARGETS PlatoParser PlatoExceptions PlatoConsole PlatoUtilities EXPORT PlatoEngine
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)
target_include_directories(PlatoParser PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${LIBLESS_HEADERS} ${Parser_HEADERS} ${Exceptions_HEADERS} ${Console_HEADERS} ${Utilities_HEADERS} DESTINATION include)
install( FILES math_parser/tinyexpr.h DESTINATION include/math_parser)

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
