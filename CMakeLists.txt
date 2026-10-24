cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(RPMegaRacer C CXX ASM)

# Option: the OPL2 on the RIA (default), or the FPGA sound card.
option(USE_NATIVE_OPL2 "Use the RIA native OPL2 support" ON)

# Track tools run in the project's .venv (pip install -r requirements.txt).
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python)
    set(TRACK_PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python)
else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(TRACK_PYTHON ${Python3_EXECUTABLE})
endif()

add_executable(RPMegaRacer)
rp6502_map(RPMegaRacer src/xram.h "XRAM_[A-Z_]*")

rp6502_asset(RPMegaRacer XRAM(XRAM_CAR_IMG) images/RedRacer.bin)
rp6502_asset(RPMegaRacer help src/help.txt)
rp6502_asset(RPMegaRacer DEMO.BIN music/DEMO.BIN)
rp6502_asset(RPMegaRacer title_tiles.bin images/title_tiles.bin)
rp6502_asset(RPMegaRacer title_map.bin images/title_map.bin)

# Each track: the map and tiles as drawn, plus the walls, grass and racing
# line that tools/make_track.py works out from them.
foreach(n 01 02 03)
    set(src ${CMAKE_CURRENT_SOURCE_DIR}/tracks/track${n})
    set(out ${CMAKE_CURRENT_BINARY_DIR}/track${n})
    add_custom_command(
        OUTPUT ${out}/walls.bin ${out}/grass.bin ${out}/path.bin
        COMMAND ${TRACK_PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/tools/make_track.py ${src} ${out}
        DEPENDS tools/make_track.py tools/trackdata.py
            ${src}/map.bin ${src}/tiles.bin ${src}/collision.bin
            ${src}/properties.bin ${src}/waypoints.json
        VERBATIM
    )
    rp6502_asset(RPMegaRacer track${n}_map.bin ${src}/map.bin)
    rp6502_asset(RPMegaRacer track${n}_tiles.bin ${src}/tiles.bin)
    rp6502_asset(RPMegaRacer track${n}_walls.bin ${out}/walls.bin)
    rp6502_asset(RPMegaRacer track${n}_grass.bin ${out}/grass.bin)
    rp6502_asset(RPMegaRacer track${n}_path.bin ${out}/path.bin)
endforeach()

rp6502_executable(RPMegaRacer DATA default RESET default)
rp6502_web(RPMegaRacer CONFIG [[
    title: 'RP Mega Racer',
]])

target_include_directories(RPMegaRacer PRIVATE src)
target_compile_options(RPMegaRacer PRIVATE -Os -Wall)
if(USE_NATIVE_OPL2)
    target_compile_definitions(RPMegaRacer PRIVATE USE_NATIVE_OPL2)
endif()

target_sources(RPMegaRacer PRIVATE
    src/main.c
    src/gfx.c
    src/hud.c
    src/input.c
    src/track.c
    src/trig.c
    src/car.c
    src/race.c
    src/ai.c
    src/opl.c
    src/instruments.c
    src/sound.c
)

# Gamepad mapper: writes JOYSTICK.DAT for the game to read.
add_executable(gamepad_mapper)
rp6502_executable(gamepad_mapper DATA default RESET default)
target_include_directories(gamepad_mapper PRIVATE src)
target_sources(gamepad_mapper PRIVATE src/gamepad_mapper.c)
