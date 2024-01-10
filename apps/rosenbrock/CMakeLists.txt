
SET(SRCS RosenbrockMain.cpp)
SET(HDRS Plato_RosenbrockApp.hpp)

add_executable(RosenbrockProxy ${SRCS})
target_link_libraries(RosenbrockProxy PlatoInterface PlatoDataLayer PlatoOptimize Boost::serialization)

install( TARGETS RosenbrockProxy EXPORT PlatoEngine DESTINATION bin )

SET(SRCS LinearOperationMain.cpp Plato_LinearOperationApp.cpp)
SET(HDRS Plato_LinearOperationApp.hpp)

# actual target:
add_executable(LinearOperationProxy ${SRCS})
target_link_libraries(LinearOperationProxy PlatoInterface PlatoDataLayer PlatoOptimize Boost::serialization)

install( TARGETS LinearOperationProxy EXPORT PlatoEngine DESTINATION bin )
 
