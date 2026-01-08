cmake_minimum_required(VERSION 3.18)

add_subdirectory(tools)

set(LLVM_MOS_PLATFORM rp6502)
find_package(llvm-mos-sdk REQUIRED)

project(RPMegaRacer C CXX ASM)

add_executable(RPMegaRacer)
rp6502_asset(RPMegaRacer 0x10000 images/RedRacer.bin)
rp6502_executable(RPMegaRacer
    RedRacer.bin.rp6502
    DATA file
    RESET file
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.hlp
)
target_sources(RPMegaRacer PRIVATE
    src/main.c
    src/player.c
    src/input.c
    src/opl.c
)
