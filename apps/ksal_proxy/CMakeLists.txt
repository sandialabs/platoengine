
SET(SRCS KsalProxyMain.cpp)
SET(HDRS Plato_KsalProxyApp.hpp)

add_executable(KsalProxy ${SRCS} ${HDRS})
target_link_libraries(KsalProxy PUBLIC PlatoOptimize PlatoInterface)

install( TARGETS KsalProxy EXPORT PlatoEngine DESTINATION bin )
