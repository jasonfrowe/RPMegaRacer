### Phase 1: Environment & Boilerplate
- [x] **Project Setup:** Configure `CMakeLists.txt` for `mos-rp6502-clang`.
- [x] **RIA Integration:** Set up the standard library hooks for the RP6502 (XRAM access, register mapping).
- [x] **Input System Port:** Port your existing input system (USB Gamepad/Keyboard) into the new project.
- [x] **Video Initialization:** Initialize the RIA to Mode 2 (Tiles) for the background and Mode 4 (Sprites) for the cars.
- [ ] **XRAM Memory Map:** Define clear boundaries for:
    - [ ] Tilemap data
    - [ ] Tile bitmaps
    - [x] Sprite bitmaps
    - [x] OPL2 register buffers/music data

### Phase 2: Graphics & Asset Pipeline
- [x] **Sprite Sheet Creation:** Create a 16x16 car sprite.
    - *Note:* If using the RIA's **affine transformation** (rotation), you only need *one* frame. If doing software rotation, you'll need 16 or 32 frames of rotation.
- [ ] **Tilemap Design:** Create a track in *Tiled* (e.g., 40x30 tiles of 8x8 size).
- [x] **Asset Conversion Script:** Write/modify a Python script to convert PNGs/TMX files into raw header files or binary blobs compatible with the RIA XRAM layout.

### Phase 3: Physics & Movement (C / Fixed-Point)
Since you are using LLVM-MOS, you can use `structs` to manage car state, which will make the code much cleaner than pure assembly.
- [x] **State Structure:** Define a `Car` struct (x, y, velocity, angle, acceleration).
- [x] **Fixed-Point Library:** Implement a basic 16.16 or 8.8 fixed-point math system for smooth movement.
- [x] **Steering Logic:**
    - Map Left/Right input to change the `angle` variable.
    - Map Accelerate to apply force along the vector defined by `angle`.
- [x] **Centripetal Force / Friction:** Implement "sideways" friction so the car can drift. (Velocity shouldn't immediately match the angle of the car).

### Phase 4: Track Logic & AI
- [ ] **Tile Collision Map:** Create a 1-bit or 2-bit map in XRAM to define "Track," "Grass," and "Wall."
- [ ] **Collision Response:** 
    - Grass: Multiply velocity by 0.7 (slow down).
    - Wall: Bounce or stop (and trigger an OPL2 "crunch" sound).
- [ ] **Waypoint System:** Create an array of `(x, y)` waypoints for AI drones.
- [ ] **AI Steering:** Write a function that makes AI cars steer toward the next waypoint once they are within a certain radius of the current one.
- [ ] **Lap Logic:** Place "Checklines" (invisible sectors) to ensure players drive the whole track before a lap is counted.

### Phase 5: OPL2 Audio Implementation
This is the heart of your "Mega" version.
- [x] **Driver Integration:** Integrate your `RP6502_OPL2` code.
- [ ] **Dynamic Engine Hum:**
    - Use one OPL2 channel for the player's engine.
    - Map the car's `velocity` to the OPL2 **F-Number (frequency)**. As the car goes faster, the pitch of the FM synth rises.
- [ ] **SFX Synthesis:** Create FM patches for:
    - Tire screech (Noise channel or high-mod frequency).
    - Crash/Wall hit (Explosive percussive sound).
    - Lap completion "Ding."
- [x] **Music Engine:** A simple VGM or custom tracker player to handle the "Shop" or "Menu" music.

### Phase 6: The "Super Sprint" UI & Meta
- [ ] **The "Shop" Screen:** Implement the between-race upgrade menu.
    - [ ] **Traction:** Reduces sideways sliding.
    - [ ] **Turbo:** Increases acceleration.
    - [ ] **Top Speed:** Increases the max velocity cap.
- [ ] **HUD:** Use the RIA's "overlay" or a dedicated tile layer to display:
    - Lap count (e.g., "LAP 1/3")
    - Rank (1st, 2nd, etc.)
    - Collected Wrenches/Points.

### Phase 7: Optimization & Polish
- [ ] **Affine Sprite Benchmarking:** If using the RIA's hardware rotation, test how many 16x16 affine sprites you can draw before the scanline limit is hit. 
- [ ] **Sprite-to-Sprite Collision:** Basic circle-based collision so cars can bump each other off-course.
- [ ] **Visual Juice:** Add small "smoke" or "dust" sprites that spawn behind cars when they drift or drive on dirt.
