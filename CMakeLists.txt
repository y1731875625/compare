cmake_minimum_required(VERSION 3.18)
project(text LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/QT/6.8.2/mingw_64")   # 你的 Qt 路径
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Charts Multimedia SerialPort Network Concurrent)

# -------------------- 源文件收集 --------------------
set(SOURCE_DIRS
    ./src
    ./src/UI
)

foreach(dir ${SOURCE_DIRS})
    aux_source_directory(${dir} srcs)
endforeach()

# -------------------- 编译选项（MSVC UTF-8） --------------------
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# -------------------- 生成可执行文件 --------------------
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
add_executable(main WIN32 ${srcs})

# -------------------- 仅链接 Qt6 --------------------
target_link_libraries(main
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Charts
    Qt6::Multimedia
    Qt6::SerialPort
    Qt6::Network
    Qt6::Concurrent
)

set_target_properties(main PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE ON
)

# ==================== 自动部署 Qt DLL ====================
# 方法1：使用 windeployqt（推荐，生成后自动复制）
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${CMAKE_PREFIX_PATH}/bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET main POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations      # 不复制翻译文件
                --no-system-d3d-compiler # 不复制 d3d 编译器
                "$<TARGET_FILE:main>"
            COMMENT "Deploying Qt runtime files to output directory"
        )
    else()
        message(WARNING "windeployqt not found, Qt DLLs will not be automatically deployed.")
    endif()
endif()