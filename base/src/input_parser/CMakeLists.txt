set(SRCS Plato_FileList.cpp
         Plato_InputEnumTypes.cpp)

set(HDRS Plato_BlockStructRule.hpp
         Plato_EnumTypeHelpers.hpp
         Plato_InputBlockStruct.hpp
         Plato_InputBlocks.hpp
         Plato_EnumParser.hpp
         Plato_InputEnumTypes.hpp
         Plato_InputParser.hpp
         Plato_RestoreBoostNvccWarnings.hpp
         Plato_SuppressBoostNvccWarnings.hpp )

add_library(PlatoInputParser ${SRCS} ${HDRS})
target_link_libraries(PlatoInputParser PUBLIC PlatoParser Boost::headers)
target_include_directories(PlatoInputParser PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoInputParser EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib )

target_include_directories(PlatoInputParser PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HDRS} DESTINATION include )

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
