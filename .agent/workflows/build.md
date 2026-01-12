---
description: Build the RPMegaRacer project using CMake presets
---

# Build Project

This workflow configures and builds the project using the "native" target preset by default, which is suitable for the `rp6502` platform with native OPL2 support.

1. Configure the project
   ```bash
   cmake --preset target-native
   ```

2. Build the project
   ```bash
   cmake --build --preset build-native
   ```

> [!TIP]
> To build for the FPGA target instead, use:
> `cmake --preset target-fpga`
> followed by:
> `cmake --build --preset build-fpga`
