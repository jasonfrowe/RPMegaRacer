cmake_minimum_required(VERSION 3.18)

add_subdirectory(tools)

set(LLVM_MOS_PLATFORM rp6502)
find_package(llvm-mos-sdk REQUIRED)

project(RPMegaRacer C CXX ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the option (Default is ON/Native) 
option(USE_NATIVE_OPL2 "Use the RIA native OPL2 support" ON)

add_executable(RPMegaRacer)

# Memory-mapped assets (loaded directly into RAM/XRAM at boot)
rp6502_asset(RPMegaRacer 0x10000 images/RedRacer.bin)

# Named ROM assets - accessible as ROM:name at runtime
rp6502_asset(RPMegaRacer help src/main.hlp)
rp6502_asset(RPMegaRacer DEMO.BIN music/DEMO.BIN)
rp6502_asset(RPMegaRacer title_tiles.bin images/title_tiles.bin)
rp6502_asset(RPMegaRacer title_map.bin images/title_map.bin)
rp6502_asset(RPMegaRacer track01_map.bin tracks/track01/map.bin)
rp6502_asset(RPMegaRacer track01_tiles.bin tracks/track01/tiles.bin)
rp6502_asset(RPMegaRacer track01_collision.bin tracks/track01/collision.bin)
rp6502_asset(RPMegaRacer track01_properties.bin tracks/track01/properties.bin)
rp6502_asset(RPMegaRacer track01_waypoints.bin tracks/track01/waypoints.bin)
rp6502_asset(RPMegaRacer track02_map.bin tracks/track02/map.bin)
rp6502_asset(RPMegaRacer track02_tiles.bin tracks/track02/tiles.bin)
rp6502_asset(RPMegaRacer track02_collision.bin tracks/track02/collision.bin)
rp6502_asset(RPMegaRacer track02_properties.bin tracks/track02/properties.bin)
rp6502_asset(RPMegaRacer track02_waypoints.bin tracks/track02/waypoints.bin)
rp6502_asset(RPMegaRacer track03_map.bin tracks/track03/map.bin)
rp6502_asset(RPMegaRacer track03_tiles.bin tracks/track03/tiles.bin)
rp6502_asset(RPMegaRacer track03_collision.bin tracks/track03/collision.bin)
rp6502_asset(RPMegaRacer track03_properties.bin tracks/track03/properties.bin)
rp6502_asset(RPMegaRacer track03_waypoints.bin tracks/track03/waypoints.bin)

rp6502_executable(RPMegaRacer
    DATA file
    RESET file
)
target_sources(RPMegaRacer PRIVATE
    src/main.c
    src/player.c
    src/input.c
    src/opl.c
    src/instruments.c
    src/track.c
    src/sound.c
    src/layer2.c
    src/ai.c
    src/collision.c
    src/hud.c
    src/racelogic.c
)

if(USE_NATIVE_OPL2)
    target_compile_definitions(RPMegaRacer PRIVATE USE_NATIVE_OPL2)
    message(STATUS "Targeting: Native RIA OPL2")
else()
    message(STATUS "Targeting: FPGA TinyFPGA Sound Card")
endif()

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