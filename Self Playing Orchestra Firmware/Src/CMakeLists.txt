cmake_minimum_required(VERSION 4.4)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(ARM_GCC arm-none-eabi-gcc REQUIRED)
set(CMAKE_C_COMPILER "${ARM_GCC}")

project(Orchestra C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
set(TIVAWARE_PATH "$ENV{HOME}/ti/TivaWare")

add_executable(Orchestra
        SPI_TESTBENCH.c
        startup_gcc.c
        DelayTimer.c
        Latch.c
        PLL.c
        PortF.c
        SPI.c
        UART1.c
        Uart.c
)

target_include_directories(Orchestra PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${TIVAWARE_PATH}"
)

target_compile_options(Orchestra PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -ffunction-sections
        -fdata-sections
        -Wall
        -Wextra
)

target_link_options(Orchestra PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -nostartfiles
        --specs=nosys.specs
        "-T${CMAKE_CURRENT_SOURCE_DIR}/TM4C123GH6PM.ld"
        -Wl,-e,ResetISR
        -Wl,--gc-sections
)

add_custom_command(TARGET Orchestra POST_BUILD
        COMMAND "${ARM_OBJCOPY}" -O ihex
        "$<TARGET_FILE:Orchestra>"
        "${CMAKE_CURRENT_BINARY_DIR}/Orchestra.hex"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Orchestra.hex"
        COMMENT "Creating Orchestra.hex"
)