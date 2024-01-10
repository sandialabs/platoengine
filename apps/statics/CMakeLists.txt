
add_executable(PlatoStatics main.cpp)
target_link_libraries(PlatoStatics PlatoInterface PlatoPenaltyModels PlatoAnalyze)

install( TARGETS PlatoStatics EXPORT PlatoEngine DESTINATION bin )
