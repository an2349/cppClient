cmake_minimum_required(VERSION 4.0)
project(Client)

set(CMAKE_CXX_STANDARD 20)

add_executable(Client Client.cpp
        Utils.cpp
        Utils.h
        MultiPart.cpp
        MultiPart.h
        HttpService.cpp
        HttpService.h
        FileService.cpp
        FileService.h
        config.h)
