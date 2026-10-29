cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Cmake_2_Projects_Template)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# ============================================================
# 用户 BSP 代码：User/Inc 放头文件，User/Src 放 .c
#
# 用 GLOB_RECURSE + CONFIGURE_DEPENDS 自动收集源文件：
#   - User/Src 下新增 .c 后无需改本文件，cmake 会自动重新配置
#   - 会递归包含子目录（如 User/Src/led/led.c）
#
# 若不希望用 GLOB（CMake 官方更推荐显式列出），可改为：
#   target_sources(${CMAKE_PROJECT_NAME} PRIVATE
#       ${CMAKE_CURRENT_SOURCE_DIR}/User/Src/bsp_led.c
#   )
# ============================================================
file(GLOB_RECURSE USER_SOURCES CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/User/Src/*.c
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${USER_SOURCES}

    # CubeMX 未启用 UART 时，手动补 HAL UART 驱动（User 下的 BSP 用到了 UART_HandleTypeDef）
    # 若以后在 CubeMX 中启用 USARTx，该文件会被自动加入 cmake/stm32cubemx/CMakeLists.txt，此行可删除
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
