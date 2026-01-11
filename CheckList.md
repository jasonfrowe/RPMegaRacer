### **Phase 1: Control & State Foundations**
*   [x] **Input Remapping:**
    *   Map `GP_BTN_B` (and a keyboard key like 'R') to `ACTION_RESCUE`.
    *   Map `GP_BTN_START` to a new `ACTION_PAUSE`.
*   [ ] **Game State Machine:** Implement a `current_state` variable in `main.c`:
    *   `STATE_TITLE`: Waiting for Start.
    *   `STATE_COUNTDOWN`: The 3-2-1 sequence.
    *   `STATE_RACING`: Active physics and AI.
    *   [ ] **Pause State:** If `ACTION_PAUSE` is hit, stop all physics updates but keep OPL2 music running (perhaps at lower volume).
    *   `STATE_FINISHED`: Player crosses Lap 5.
    *   `STATE_GAMEOVER`: Showing results/leaderboard.
*   [ ] **Race Manager:**
    *   Track `player_lap_count` and `ai_lap_counts[3]`.
    *   Trigger `STATE_FINISHED` when any car hits Lap 5.

### **Phase 2: HUD & UI Expansion**
*   [ ] **The Race Timer:**
    *   Create a `uint32_t race_frames` counter that starts at `STATE_RACING`.
    *   Display as `MM:SS:CC` (Minutes:Seconds:Centiseconds) in the HUD.
*   [ ] **Title Screen:**
    *   Use Mode 1 (Text) or a static Tile map on Plane 1 to show a "MEGA RACER" logo.
    *   Add "Press Start to Race."
*   [ ] **Ranking Logic:**
    *   Every second, calculate who is 1st through 4th based on (Laps * WaypointID).
    *   Display "POS: 1/4" in the HUD.

### **Phase 3: Gameplay "Juice" & Mechanics**
*   [ ] **AI Rubber Banding:** 
    *   If Player is 1st: Decrease `AI_THRUST_SHIFT` (AI gets faster/aggressive).
    *   If Player is 4th: Increase `AI_THRUST_SHIFT` (AI slows down to let player catch up).
*   [ ] **High Impact Explosions:**
    *   If `abs(velocity)` is near max and `terrain == TERRAIN_WALL`:
    *   Trigger `play_psg_explosion()`.
    *   Swap car sprite to an "explosion" frame (or flicker it).
    *   Force a `rescue_player()` after a 1-second delay.
*   [ ] **Rescue Helicopter:**
    *   When `rescue_player` is triggered, don't just teleport.
    *   Spawn a Helicopter sprite at the car's current position, "lift" it, and move it to the waypoint before disappearing.
*   [ ] **Pickups & Hazards (Sprites):**
    *   [ ] **Wrenches:** Collect to spend in the shop (Phase 6).
    *   [ ] **Oil Spills:** If car touches, force `p->angle += 32` (random spin) and kill traction for 30 frames.

### **Phase 4: Content & Environment**
*   [ ] **Scenery Layer (Plane 1):**
    *   Initialize a second Tile Plane.
    *   Use it for "Overhanging" graphics: Bridges, tree tops, or stadium lights that the car drives *under*.
*   [ ] **Closing Gates:**
    *   Use an Affine Sprite (large scale) or an animated tile.
    *   Open/Close based on the `race_frames` timer.
*   [ ] **Additional Maps:**
    *   Create `track2.bin`, `track3.bin`.
    *   Implement a loader that clears XRAM and loads new assets between races.

### **Phase 5: Meta & Persistence**
*   [ ] **High Score System:**
    *   Create a `scores.dat` file.
    *   Store the fastest total race time and the fastest single lap time.
*   [ ] **The "Super Sprint" Shop:**
    *   A screen between tracks.
    *   Spend collected wrenches on: `Higher Top Speed`, `Faster Acceleration`, `Super Traction`.

---

### **Missing "Essentials" to add to your list:**

1.  **Race Results Screen:** Don't just reset! Show a table: "1st: Player (Time), 2nd: AI 1..."
2.  **Soundscape Variety:** 
    *   A different OPL2 track for the Title Screen vs. the Race.
    *   A "Final Lap" music tempo increase (if OPL2 driver supports it).
3.  **Visual Polish - "Tire Smoke":**
    *   When drifting or accelerating from 0, spawn tiny "Cloud" sprites behind the rear tires. 
    *   Since you have spare sprites, this adds a huge amount of "feel."
4.  **AI Personalities:**
    *   Give the 3 AI cars different `AI_TURN_SPEED` or `AI_MAX_ACCEL` values so they don't stay in a perfect line.

---

### **Immediate Recommendation: The "Pause" and "Rescue" Buttons**
Since you are about to edit `input.c`, add a **Cooldown** to the Rescue button. If the player mashes it, they could glitch the lap counter.

**Logic for `player.c`:**
```c
if (is_action_pressed(0, ACTION_RESCUE) && rescue_cooldown == 0) {
    rescue_player(p);
    rescue_cooldown = 120; // Prevent reuse for 2 seconds
}
if (rescue_cooldown > 0) rescue_cooldown--;
```



### Phase 1: Environment & Boilerplate
- [x] **Project Setup:** Configure `CMakeLists.txt` for `mos-rp6502-clang`.
- [x] **RIA Integration:** Set up the standard library hooks for the RP6502 (XRAM access, register mapping).
- [x] **Input System Port:** Port your existing input system (USB Gamepad/Keyboard) into the new project.
- [x] **Video Initialization:** Initialize the RIA to Mode 2 (Tiles) for the background and Mode 4 (Sprites) for the cars.
- [x] **XRAM Memory Map:** Define clear boundaries for:
    - [x] Tilemap data
    - [x] Tile bitmaps
    - [x] Sprite bitmaps
    - [x] OPL2 register buffers/music data

### Phase 2: Graphics & Asset Pipeline
- [x] **Sprite Sheet Creation:** Create a 16x16 car sprite.
    - *Note:* If using the RIA's **affine transformation** (rotation), you only need *one* frame. If doing software rotation, you'll need 16 or 32 frames of rotation.
- [x] **Tilemap Design:** Create a track in *Tiled* (e.g., 40x30 tiles of 8x8 size).
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
- [x] **Tile Collision Map:** Create a 1-bit or 2-bit map in XRAM to define "Track," "Grass," and "Wall."
- [x] **Collision Response:** 
    - Grass: Multiply velocity by 0.7 (slow down).
    - Wall: Bounce or stop (and trigger an OPL2 "crunch" sound).
- [x] **Waypoint System:** Create an array of `(x, y)` waypoints for AI drones.
- [x] **AI Steering:** Write a function that makes AI cars steer toward the next waypoint once they are within a certain radius of the current one.
- [x] **Lap Logic:** Place "Checklines" (invisible sectors) to ensure players drive the whole track before a lap is counted.

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
    - [x] Lap count (e.g., "LAP 1/3")
    - [ ] Rank (1st, 2nd, etc.)
    - [ ] Collected Wrenches/Points.

### Phase 7: Optimization & Polish
- [x] **Affine Sprite Benchmarking:** If using the RIA's hardware rotation, test how many 16x16 affine sprites you can draw before the scanline limit is hit. 
- [ ] **Sprite-to-Sprite Collision:** Basic circle-based collision so cars can bump each other off-course.
- [ ] **Visual Juice:** Add small "smoke" or "dust" sprites that spawn behind cars when they drift or drive on dirt.
