cmake_minimum_required(VERSION 3.18)

add_subdirectory(tools)

set(LLVM_MOS_PLATFORM rp6502)
find_package(llvm-mos-sdk REQUIRED)

project(RPMegaRacer C CXX ASM)

# Define the option (Default is ON/Native) 
option(USE_NATIVE_OPL2 "Use the RIA native OPL2 support" ON)

if(USE_NATIVE_OPL2)
    add_definitions(-DUSE_NATIVE_OPL2)
    message(STATUS "Targeting: Native RIA OPL2")
else()
    message(STATUS "Targeting: FPGA TinyFPGA Sound Card")
endif()

add_executable(RPMegaRacer)
rp6502_asset(RPMegaRacer 0x10000 images/RedRacer.bin)
rp6502_asset(RPMegaRacer 0x10850 images/Track_Map_A.bin)
rp6502_asset(RPMegaRacer 0x11450 images/Track_A_tiles.bin)
rp6502_executable(RPMegaRacer
    RedRacer.bin.rp6502
    Track_Map_A.bin.rp6502
    Track_A_tiles.bin.rp6502
    DATA file
    RESET file
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.hlp
)
target_sources(RPMegaRacer PRIVATE
    src/main.c
    src/player.c
    src/input.c
    src/opl.c
    src/instruments.c
    src/track.c
    src/sound.c
    src/ai.c
    src/collision.c
    src/hud.c
    src/racelogic.c
)

# --- Gamepad Mapper Utility ---
# This creates a separate binary called gamepad_mapper.rp6502
add_executable(gamepad_mapper)

# This uses the simple loader macro (no heavy assets needed)
rp6502_executable(gamepad_mapper
DATA file
RESET file
)

target_sources(gamepad_mapper PRIVATE
    src/gamepad_mapper.c
)