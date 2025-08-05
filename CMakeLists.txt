cmake_minimum_required(VERSION 3.16)

# 设置vcpkg工具链文件
# set(CMAKE_TOOLCHAIN_FILE "C:/work/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

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

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Svg)

# 输出一些调试信息
message(STATUS "Qt5Widgets_INCLUDE_DIRS: ${Qt5Widgets_INCLUDE_DIRS}")
message(STATUS "Qt5Svg_INCLUDE_DIRS: ${Qt5Svg_INCLUDE_DIRS}")

# 添加可执行文件
add_executable(ai-desktop-helper
    src/main.cpp
    src/floatingball.cpp
    src/floatingball.h
    src/trayicon.cpp
    src/trayicon.h
    src/screenmonitor.cpp
    src/screenmonitor.h
    src/networkmanager.cpp
    src/networkmanager.h
    src/common.h
    src/settingsdialog/settingsdialog.cpp
    src/settingsdialog/settingsdialog.h
    src/settingsdialog/appFilterWidget.cpp
    src/settingsdialog/appFilterWidget.h
    src/settingsdialog/recordingwidget.cpp
    src/settingsdialog/recordingwidget.h
    src/settingsdialog/settingsnavigationbar.cpp
    src/settingsdialog/settingsnavigationbar.h
    resources.qrc
)

# 链接库
target_link_libraries(ai-desktop-helper
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Svg
    version
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
) 