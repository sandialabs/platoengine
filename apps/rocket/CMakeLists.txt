
SET(SRCS RocketDesignMain.cpp
                    Plato_RocketDesignApp.cpp
                    )
SET(HDRS Plato_RocketDesignApp.hpp)

add_executable(RocketDesign ${SRCS})
target_link_libraries(RocketDesign PlatoOptimize PlatoProxy PlatoInterface)

install( TARGETS RocketDesign EXPORT PlatoEngine DESTINATION bin )
            
