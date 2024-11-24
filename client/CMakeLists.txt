cmake_minimum_required(VERSION 3.10)
project(Client)

set(SOURCES src/main.cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS system thread)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories($(CMAKE_CURRENT_SOURCE_DIR)/includes)
else()
    message(FATAL_ERROR "Boost not found!")
endif()

add_executable(Client ${SOURCES})

target_link_libraries(Client ${Boost_LIBRARIES})

