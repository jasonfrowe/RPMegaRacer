#include <rp6502.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "layer2.h"
#include "constants.h"

void load_plane2(void)
{
    // Load Title Map and Tiles dynamic from filesystem (assets/)
    int fd;

    // 1. Load Tile Data (Graphics)
    fd = open("assets/title_tiles.bin", O_RDONLY);
    if (fd >= 0) {
        // read_xram reads directly from file descriptor to XRAM address
        read_xram(TITLE_DATA, TITLE_DATA_SIZE, fd);
        close(fd);
        printf("Loaded assets/title_tiles.bin\n");
    } else {
        printf("Error: Could not open assets/title_tiles.bin\n");
    }

    // 2. Load Tile Map (Layout)
    fd = open("assets/title_map.bin", O_RDONLY);
    if (fd >= 0) {
        read_xram(TITLE_MAP_ADDR, TITLE_MAP_SIZE, fd);
        close(fd);
        printf("Loaded assets/title_map.bin\n");
    } else {
        printf("Error: Could not open assets/title_map.bin\n");
    }
}
