cmake_minimum_required(VERSION 3.10)

project(KdrobotCppLibs)

set(CMAKE_INSTALL_PREFIX "/usr/local/KdrobotCppLibs")

add_subdirectory(modules)

message(STATUS "配置完成，使用'make install'安装")
message(STATUS "当前安装目录'${CMAKE_INSTALL_PREFIX}'")
message(STATUS "不提供卸载功能，注意安装位置应易于删除")

