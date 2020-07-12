cmake_minimum_required(VERSION 2.6)
project(cellrobot_app)

# set build env
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_SYSTEM_NAME Linux)

## ****** x86 make options ************ ###
set(CMAKE_SYSTEM_PROCESSOR x86-desktop)
#set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "-D_REENTRANT ${CMAKE_C_FLAGS}")
# reentrant
set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_FLAGS "-Wcpp ${CMAKE_CXX_FLAGS}")
SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_FIND_ROOT_PATH "/usr/bin")

## ****** arm make options ************ ###
set(CMAKE_SYSTEM_PROCESSOR arm)
#SET(CMAKE_SYSROOT "/home/robot/toolSoft/host/arm-buildroot-linux-gnueabihf/sysroot")
#SET(tools "/home/robot/toolSoft/host")
#set(CMAKE_C_COMPILER ${tools}/bin/arm-linux-gcc)
#set(CMAKE_CXX_COMPILER ${tools}/bin/arm-linux-g++)
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMKAE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
#SET(CMAKE_FIND_ROOT_PATH "/home/robot/toolSoft/host/arm-buildroot-linux-gnueabihf")


MESSAGE(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER})
MESSAGE(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
MESSAGE(STATUS "CMAKE_FIND_ROOT_PATH: " ${CMAKE_FIND_ROOT_PATH})

add_executable(debug_app
		src/app.cpp
		src/db_libs/select.cpp
		src/db_libs/table_opration.cpp
		src/db_libs/db_entry.cpp
		src/download/down_load_file.cpp
		src/download/update_version.cpp
		src/download/forlder_util.cpp
		src/download/infor_unpacket.cpp
		src/download/version_infor.cpp)

include_directories(third_party/spdlog/include)
include_directories(third_party/json/single_include)
include_directories(third_party/concurrentqueue)
include_directories(/home/robot/toolSoft/host/arm-buildroot-linux-gnueabihf./lib)

target_link_libraries(debug_app pthread rt event event_pthreads)
