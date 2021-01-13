cmake_minimum_required(VERSION 3.10)

project(KdrobotCppLibs)

if (WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/KdrobotCppLibs")
    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    endif ()
elseif (linux)
    set(CMAKE_INSTALL_PREFIX "/usr/local/KdrobotCppLibs")
    if (GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    endif ()
endif ()

add_subdirectory(modules)

message(STATUS "Configuration complete use 'make install' to install")
message(STATUS "Install prefix: '${CMAKE_INSTALL_PREFIX}'")
message(STATUS "Environment variables that should be added:
    PATH += ${CMAKE_INSTALL_PREFIX}/bin/${CMAKE_BUILD_TYPE}
    CMAKE_PREFIX_PATH += ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_BUILD_TYPE}")
