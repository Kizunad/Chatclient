cmake_minimum_required(VERSION 3.10)
project(Client)

# 设置源文件
set(SOURCES src/main.cpp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找 Boost 库
find_package(Boost REQUIRED COMPONENTS system thread)

# 检查 Boost 是否找到
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes) # 修正语法
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# 添加可执行文件
add_executable(Client ${SOURCES})

# 链接 Boost 库
target_link_libraries(Client ${Boost_LIBRARIES})
