# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Clean CMakeLists.txt for project
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pull in Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(ws2812_project C CXX ASM)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(ws2812_project
    ws2812.c
)

# Generate the PIO header
pico_generate_pio_header(ws2812_project ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Link libraries
target_link_libraries(ws2812_project
    pico_stdlib
    hardware_pio
)

# Generate extra output formats (UF2, binary)
pico_add_extra_outputs(ws2812_project)
