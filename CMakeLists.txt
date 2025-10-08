cmake_minimum_required(VERSION 3.10)
project(gcloud_engine_for_chessbase)

set(CMAKE_CXX_STANDARD 20)

#Packages
find_package(LIBSSH REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

#Source code for except main methods, allowing for substition for test purposes
add_library(source_lib
    engine/engine.cpp
    engine/engine.h
    engine/configuration.cpp
    engine/configuration.h
    config/config.cpp
    config/config.h
    shared/gcloud.cpp
    shared/ssh.cpp
    shared/os_specific.cpp
    shared/file.cpp
    shared/shared.h
)
target_link_libraries(source_lib PRIVATE ssh yaml-cpp::yaml-cpp)

#Source code for main methods and targets for Config and Engine binaries
set(target_sources_engine
    engine/main.cpp
)
set(target_sources_config
    config/main.cpp
)

add_executable(gcloud_engine ${target_sources_engine})
target_link_libraries(gcloud_engine PRIVATE source_lib ssh yaml-cpp::yaml-cpp)

add_executable(gcloud_config ${target_sources_config})
target_link_libraries(gcloud_config PRIVATE source_lib ssh yaml-cpp::yaml-cpp)

#include directories
target_include_directories(source_lib PRIVATE shared config engine)
target_include_directories(gcloud_engine PRIVATE shared config engine)
target_include_directories(gcloud_config PRIVATE shared config engine)
