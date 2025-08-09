# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)


project(transmissor-lora C CXX ASM) 
pico_sdk_init()


#include_directories(${CMAKE_SOURCE_DIR}/libs/include) # Adiciona o diretório 'libs/include' à lista global de diretórios de headers 

add_executable(${PROJECT_NAME}  
        transmissor-lora.c # Código principal em C
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})  # Adiciona o diretório raiz como include privado apenas para o target atual

target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})