cmake_minimum_required(VERSION 3.10)

project(KdrobotCppLibs)

if (WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/KdrobotCppLibs")
    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    else ()
        message(ERROR "Unknown Compiler")
    endif ()
elseif (UNIX)
    set(CMAKE_INSTALL_PREFIX "/usr/local/KdrobotCppLibs")
    if (GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    endif ()
endif ()

add_subdirectory(modules)

message(STATUS "Configuration complete use 'make install' to install")
message(STATUS "Install prefix: '${CMAKE_INSTALL_PREFIX}'")
if (WIN32)
    message(STATUS "Environment variables that should be added:
    PATH += ${CMAKE_INSTALL_PREFIX}/bin
    CMAKE_PREFIX_PATH += ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_BUILD_TYPE}/cmake")
elseif (UNIX)
    message(STATUS "Environment variables that should be added:
    export PATH = ${CMAKE_INSTALL_PREFIX}/bin${CMAKE_BUILD_TYPE}\${PATH:+:\${PATH}}
    export CMAKE_PREFIX_PATH = ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_BUILD_TYPE}/cmake\${CMAKE_PREFIX_PATH:+:\${CMAKE_PREFIX_PATH}}")
endif ()
