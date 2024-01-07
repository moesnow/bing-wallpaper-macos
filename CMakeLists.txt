# 设置CMake的最低版本要求
cmake_minimum_required(VERSION 3.12)

# 指定项目名称
project(bing-wallpaper-macos)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)

# 检查处理器架构
if (APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    include_directories(/opt/homebrew/include)
endif()

# 查找并添加源文件
file(GLOB SRCS "*.cpp")

# 生成目标文件
add_executable(bing-wallpaper-macos ${SRCS})

# 链接外部库
target_link_libraries(bing-wallpaper-macos curl)

# 指定头文件目录
target_include_directories(bing-wallpaper-macos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
