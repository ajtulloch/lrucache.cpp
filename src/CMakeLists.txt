# Packages
find_package(Boost 1.54 COMPONENTS chrono system REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

# lrucache executable
set(LRUCACHE_SRCS main.cpp)
add_executable(lrucache ${LRUCACHE_SRCS})
target_link_libraries(lrucache ${Boost_LIBRARIES})

set(CMAKE_BUILD_TYPE Debug)
