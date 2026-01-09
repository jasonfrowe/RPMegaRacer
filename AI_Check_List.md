# AI-Driven Cars Implementation Checklist

## Overview
Implement AI-driven opponent cars that race around the track using waypoint navigation.

## Configuration
- **Total Cars**: 4 (1 player + 3 AI)
- **Starting Line**: X=255, Y from 33-86 (start/finish at waypoint 0)
- **Coordinate System**: 0,0 = top-left, X increases right (max 512), Y increases down (max 384)
- **Starting Waypoint**: AI cars start targeting waypoint 1 (120, 50) to avoid turning backward
- **Waypoint Reach Radius**: 40 pixels (increased to prevent overshoot issues)
- **Look-Ahead Distance**: 10 pixels past waypoint
- **Random Offset**: ±10 pixels per car for natural racing lines

## Waypoints
| ID | Location | X | Y | Notes |
|----|----------|---|---|-------|
| 0 | Top-Middle (Start/Finish) | 255 | 60 | Starting point, center of track |
| 1 | Top-Right Entry | 120 | 50 | Move toward outer edge |
| 2 | Top-Right Apex | 91 | 84 | Hug inner corner |
| 3 | Right-Side Mid | 60 | 192 | Center of vertical straight |
| 4 | Bottom-Right Entry | 48 | 255 | Outer edge before corner |
| 5 | Bottom-Right Apex | 88 | 295 | Inner corner |
| 6 | Bottom-Middle | 247 | 315 | Center of bottom straight |
| 7 | Bottom-Left Entry | 375 | 328 | Outer edge before corner |
| 8 | Bottom-Left Apex | 431 | 295 | Inner corner |
| 9 | Left-Side Mid | 446 | 175 | Center of vertical straight |
| 10 | Top-Left Entry | 453 | 116 | Outer edge before corner |
| 11 | Top-Left Apex | 417 | 80 | Inner corner |

## Implementation Tasks

### Phase 1: Data Structures ✓
- [x] Create AI car structure with position, velocity, angle
- [x] Define waypoint array with coordinates
- [x] Add AI car state (current waypoint, offset)

### Phase 2: Starting Grid ✓
- [x] Position player car at (245, 76) left of starting line
- [x] Position AI cars at (245, 40/52/64) left of starting line
- [x] Set all cars facing left (angle 64) toward track
- [x] Initialize AI cars with waypoint 1 as first target (skip start/finish)
- [x] Add 10-second startup delay (600 frames)

### Phase 3: Waypoint Navigation ✓
- [x] Calculate distance to current waypoint
- [x] Switch to next waypoint when within reach radius (25px)
- [x] Calculate steering angle toward waypoint + look-ahead (10px)
- [x] Apply steering to AI car angle with turn rate limit
- [x] Counter-clockwise progression through waypoints 0→1→2→...→11→0

### Phase 4: AI Movement ✓
- [x] Apply constant forward thrust to AI cars
- [x] Use same physics as player (velocity, friction)
- [x] Apply collision detection for AI cars
- [x] Handle AI car bouncing off walls

### Phase 5: Racing Behavior (Partial)
- [x] Add random offset to each AI car's waypoint target (±10px)
- [ ] Implement speed variation (some cars faster than others)
- [ ] Add lap counter for AI cars
- [ ] Detect AI cars crossing finish line

### Phase 6: Polish ✓
- [x] Draw all AI car sprites with rotation
- [x] Update camera to follow player only
- [ ] Add visual indicators (position display, lap counter)
- [ ] Optimize AI update performance

## Technical Notes

### Waypoint Transition Logic
```c
// Check if close enough to current waypoint
float dx = ai.x - waypoint[ai.current].x;
float dy = ai.y - waypoint[ai.current].y;
float dist = sqrt(dx*dx + dy*dy);

if (dist < REACH_RADIUS) {
    ai.current = (ai.current + 1) % NUM_WAYPOINTS;
    // Generate new random offset for this waypoint
    ai.offset_x = (rand() % 20) - 10;
    ai.offset_y = (rand() % 20) - 10;
}
```

### Look-Ahead Steering
```c
// Target point is 10 pixels past waypoint
float target_x = waypoint[ai.current].x + ai.offset_x;
float target_y = waypoint[ai.current].y + ai.offset_y;

// Calculate direction to next waypoint
int next = (ai.current + 1) % NUM_WAYPOINTS;
float dx = waypoint[next].x - waypoint[ai.current].x;
float dy = waypoint[next].y - waypoint[ai.current].y;
float mag = sqrt(dx*dx + dy*dy);

// Add look-ahead
target_x += (dx / mag) * 10;
target_y += (dy / mag) * 10;

// Steer toward target
float angle_to_target = atan2(target_x - ai.x, target_y - ai.y);
```

### Starting Positions
- Player: (255, 86) - Back row, center
- AI 1: (255, 69) - Second row
- AI 2: (255, 52) - Third row  
- AI 3: (255, 35) - Front row (pole position)

## Current Status
- **Phase 1**: In Progress
- Sprite data loaded for 4 cars
- Configuration structs allocated
- Need to create AI car data structures and waypoint array
