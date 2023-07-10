cmake_minimum_required(VERSION 3.10.2)
project(BXC_SipServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ")

set(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")

set(INCLUDE_DIR /usr/local/include)
set(LIB_DIR /usr/local/lib)
include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

add_executable(BXC_SipServer
        Utils/MD5.c
        Utils/HTTPDigest.c
        SipServer.cpp
        main.cpp
        )
TARGET_LINK_LIBRARIES(BXC_SipServer osip2 osipparser2 eXosip2)





