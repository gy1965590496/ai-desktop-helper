cmake_minimum_required(VERSION 3.16)

# 设置vcpkg工具链文件
set(CMAKE_TOOLCHAIN_FILE "C:/work/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(ai-desktop-helper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置 UTF-8 编码
if(MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# 确保使用vcpkg安装的包
set(CMAKE_PREFIX_PATH "C:/work/vcpkg/vcpkg/vcpkg_installed/x64-windows")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 输出一些调试信息
message(STATUS "Qt5Widgets_INCLUDE_DIRS: ${Qt5Widgets_INCLUDE_DIRS}")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/floatingball.cpp
    src/floatingball.h
    src/trayicon.cpp
    src/trayicon.h
    resources.qrc
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    Qt5::Widgets
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 