# Motion Algorithms Documentation

## Overview

LemLib-X provides four core motion algorithms for autonomous movement:

1. **moveToPoint**: Drive straight to a target location
2. **moveToPose**: Drive to a location while facing a specific direction  
3. **turnTo**: Rotate to face a target heading
4. **follow**: Follow a pre-defined path using Pure Pursuit algorithm

All motions run asynchronously via the MotionHandler and merge error feedback using PID controllers.

## Common Concepts

### Motion Execution

```cpp
// All motions are non-blocking - code continues immediately
lemlib::moveToPoint(target, timeout);
std::cout << "Motion started!" << std::endl;  // Prints immediately

// Check if motion is running
if (lemlib::motion_handler::isMoving()) {
    std::cout << "Motion in progress" << std::endl;
}

// Wait for completion if needed
while (lemlib::motion_handler::isMoving()) {
    pros::delay(10);
}
```

### Parameters

**Common parameters across all motions:**

- `timeout`: Maximum time motion is allowed to run
  - If exceeded, motion ends even if not settled
  - Provides safety against stalled robots
  - Type: `Time` (use `5000_msec` syntax)

**Motion-specific parameters:**

- `FollowParams` - for `follow()` algorithm
- `MoveToPoseParams` - for `moveToPose()` algorithm
- Implicit defaults from `config.hpp` if not specified

### Exit Conditions

Each motion ends when **any** of these occur:
1. Exit condition satisfied (error within range for duration)
2. Timeout reached
3. Motion cancelled via `lemlib::motion_handler::cancel()`

## moveToPoint

Drive straight forward/backward to reach a target point.

### Signature

```cpp
void moveToPoint(Point target, 
                 Time timeout,
                 MoveToPointParams params = {},
                 MoveToPointSettings settings = {})
```

### Parameters

**MoveToPointParams:**
- `reversed`: Drive backwards to target (default: false)
- `lateralSlew`: Acceleration limiting for forward/backward (default: `lateral_slew`)

**MoveToPointSettings:**
- `poseGetter`: Function returning current robot Pose (default: `pose_getter`)
- `leftMotors`: Motor group for left side (default: `left_motors`)
- `rightMotors`: Motor group for right side (default: `right_motors`)

### Usage Examples

#### Basic Forward Movement

```cpp
void autonomous() {
    // Drive to point (24 inches forward, 0 inches right) with 5 second timeout
    lemlib::moveToPoint(Point(24, 0), 5000_msec);
    
    // Wait for completion
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    std::cout << "Reached target!" << std::endl;
}
```

#### Backward Movement

```cpp
void autonomous() {
    // Back up 12 inches
    lemlib::moveToPoint(Point(-12, 0), 3000_msec);
    
    // Or using params:
    lemlib::MoveToPointParams params;
    params.reversed = true;
    lemlib::moveToPoint(Point(12, 0), 3000_msec, params);
}
```

#### With Acceleration Limiting

```cpp
void autonomous() {
    lemlib::MoveToPointParams params;
    params.lateralSlew = 0.5;  // Reduce acceleration
    
    // Smooth acceleration motion
    lemlib::moveToPoint(Point(48, 0), 5000_msec, params);
}
```

### How It Works

```
1. Calculate error from current position to target
   error = target_distance - current_distance

2. Run lateral PID with error
   output = lateral_pid.update(error)

3. Apply same output to left and right motors
   left_motors.moveVoltage(output)
   right_motors.moveVoltage(output)

4. Check exit condition
   if (settled for 200ms) → motion done

5. Loop repeats every 10ms
```

### Common Issues

**Robot leaves original position but doesn't reach target:**
- Increase lateral PID kP
- Check physical drivetrain (wheels, gears)
- Verify motor power

**Robot overshoots target:**
- Reduce lateral PID kP
- Increase lateral PID kD

**Motion oscillates around target:**
- Reduce lateral PID kP
- Increase lateral PID kD (add damping)

## moveToPose

Drive to a target location while rotating to a target heading.

### Signature

```cpp
void moveToPose(Pose target,
                Time timeout,
                MoveToPoseParams params = {},
                MoveToPoseSettings settings = {})
```

### Parameters

**MoveToPoseParams:**
- `reversed`: Drive backwards (default: false)
- `maxAngularVelocity`: Max rotation speed deg/sec (default: 360)
- `earliestRotation`: Degree (0-1) when to start rotating (default: 0.5 = halfway)
- `lateralSlew`: Forward/backward acceleration limit (default: `lateral_slew`)
- `angularSlew`: Rotation acceleration limit (default: `angular_slew`)

**MoveToPoseSettings:**
- Standard settings like `poseGetter`, motor groups (same as moveToPoint)

### Usage Examples

#### Basic Drive + Rotate

```cpp
void autonomous() {
    // Drive to (24, 24) while facing 45 degrees with 5 second timeout
    lemlib::moveToPose(Pose(24, 24, 45_deg), 5000_msec);
    
    // Wait for completion
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
}
```

#### Control Rotation Timing

```cpp
void autonomous() {
    lemlib::MoveToPoseParams params;
    params.earliestRotation = 0.8;  // Start rotating 80% through motion
    
    // Drives most of the way first, then rotates
    lemlib::moveToPose(Pose(48, 0, 90_deg), 5000_msec, params);
}
```

#### Backward Motion with Rotation

```cpp
void autonomous() {
    lemlib::MoveToPoseParams params;
    params.reversed = true;
    
    // Back up to (12, 0) while facing 180 degrees
    lemlib::moveToPose(Pose(12, 0, 180_deg), 3000_msec, params);
}
```

#### Limited Rotation Speed

```cpp
void autonomous() {
    lemlib::MoveToPoseParams params;
    params.maxAngularVelocity = 180;  // Max 180 deg/sec (instead of 360)
    
    // Slower rotation for precision
    lemlib::moveToPose(Pose(24, 24, 90_deg), 5000_msec, params);
}
```

### How It Works

```
1. Calculate lateral error (distance to target XY)
   lateral_error = distance(current_pos, target_pos)

2. Calculate angular error (heading difference)
   angular_error = normalize(target_heading - current_heading)

3. If motion is far from target
   - Run lateral PID for forward/backward
   - If earliestRotation point reached, start angular PID
   
4. If motion is close to target
   - Run both lateral and angular PID simultaneously
   - Converge to exact position and heading

5. Exit when both lateral AND angular are settled
   (each has separate exit condition)

6. Loop every 10ms
```

### Tuning earliestRotation

- `0.0` = Start rotating immediately (parallel motion)
- `0.5` = Start rotating halfway (default, balanced)
- `1.0` = Finish lateral first, then rotate (serial motion)

**Use case suggestions:**
- **0.0 parallel**: For tight enclosed spaces, best reliability
- **0.5 balanced**: General purpose, balanced speed vs precision
- **1.0 serial**: Wide open field, all-lateral then all-angular

## turnTo

Rotate to face a target heading.

### Signature

```cpp
void turnTo(Angle target,
            Time timeout,
            TurnToParams params = {},
            TurnToSettings settings = {})
```

### Parameters

**TurnToParams:**
- `direction`: Force direction - `CW` (right) or `CCW` (left)
  - Default: Chooses shortest rotation direction
- `maxAngularVelocity`: Max rotation speed (default: 360 deg/sec)
- `angularSlew`: Rotation acceleration limit (default: `angular_slew`)

**TurnToSettings:**
- Standard settings like `poseGetter`, motor groups

### Usage Examples

#### Basic Rotation

```cpp
void autonomous() {
    // Rotate to face 90 degrees (right turn)
    lemlib::turnTo(90_deg, 2000_msec);
    
    // Wait for completion
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
}
```

#### Specific Rotation Direction

```cpp
void autonomous() {
    // Rotate 90 degrees forcing counter-clockwise
    lemlib::TurnToParams params;
    params.direction = lemlib::RotationDirection::CCW;
    
    lemlib::turnTo(90_deg, 2000_msec, params);
}
```

#### Limited Rotation Speed

```cpp
void autonomous() {
    lemlib::TurnToParams params;
    params.maxAngularVelocity = 180;  // Max ~180 deg/sec
    
    // Slow, controlled rotation
    lemlib::turnTo(180_deg, 3000_msec, params);
}
```

### How It Works

```
1. Calculate angular error
   error = target_heading - current_heading
   error = normalize(error)  // -180 to +180 degrees

2. Choose rotation direction (unless forced)
   - If error > 0: Rotate counter-clockwise
   - If error < 0: Rotate clockwise

3. Run angular PID controller
   output = angular_pid.update(error)

4. Apply opposite torques to motors
   left_motors.moveVoltage(-output)
   right_motors.moveVoltage(+output)

5. Check exit condition
   if (within 2 degrees for 150ms) → motion done

6. Loop every 10ms
```

### Common Issues

**Robot rotates but overshoots heading:**
- Reduce angular PID kP
- Increase angular PID kD

**Robot oscillates around target heading:**
- Reduce angular PID kP
- Increase angular PID kD

**Rotation is very slow:**
- Increase angular PID kP
- Check motor power

## follow

Follow a pre-defined path using Pure Pursuit algorithm.

### Signature

```cpp
void follow(const asset& path,
            Length lookaheadDistance,
            Time timeout,
            FollowParams params = {},
            FollowSettings settings = {})
```

### Parameters

**Path Asset:**
- Requires path data (created with path generator tools)
- Contains waypoints with positions and headings

**lookaheadDistance:**
- How far ahead of current position to look for target point
- Typical range: 6-12 inches
- Larger = smoother path, less responsive to waypoints
- Smaller = more precise but may oscillate

**FollowParams:**
- `reversed`: Drive backwards along path (default: false)
- `lateralSlew`: Acceleration limit (default: `lateral_slew`)

**FollowSettings:**
- Standard settings like motor groups, pose getter

### Usage Examples

#### Basic Path Following

```cpp
void autonomous() {
    // Follow pre-generated path with 8 inch lookahead, 10 second timeout
    lemlib::follow(path_to_goal, 8_in, 10000_msec);
    
    // Wait for completion
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
}
```

#### With Tuning

```cpp
void autonomous() {
    lemlib::FollowParams params;
    params.lateralSlew = 0.5;  // Smooth acceleration
    
    // Follow path with optimized settings
    lemlib::follow(path_to_goal, 8_in, 10000_msec, params);
}
```

#### Reverse Following

```cpp
void autonomous() {
    lemlib::FollowParams params;
    params.reversed = true;
    
    // Drive backwards along path
    lemlib::follow(path_curve, 8_in, 5000_msec, params);
}
```

### Pure Pursuit Algorithm

```
1. Calculate current position and heading

2. Find lookahead point
   - Distance 'lookaheadDistance' ahead on path curve
   - Intersection of circle(radius=lookahead) and path

3. Calculate steering to point at lookahead point
   - Lateral error = perpendicular distance to lookahead point
   - Apply lateral PID for steering

4. Apply velocity towards target (along path)
   - Blend heading from robot to lookahead point
   - Smooth path following behavior

5. Check if path is complete
   - At end of path AND heading settled
   
6. Loop every 10ms
```

### Lookahead Distance Tuning

| Distance | Behavior | Best For |
|---|---|---|
| 2-4 in | Twitchy, precise | Tight paths, high precision |
| 6-8 in | Balanced | General purpose paths |
| 10-15 in | Smooth, forgiving | Wide curves, speed |

**Rule of thumb:** Use max speed ÷ 500 = lookahead in inches

### Common Issues

**Robot oscillates left-right on path:**
- Reduce lookahead distance
- Reduce lateral PID kP

**Robot can't keep up with path:**
- Increase lookahead distance
- Increase motor power

**Robot drifts off path gradually:**
- Increase lookahead distance slightly
- Check odometry accuracy

## Comparison Table

| Motion | Purpose | Features | Exit Condition |
|---|---|---|---|
| **moveToPoint** | Drive straight forward/back | Simple, direct | Settled near target |
| **moveToPose** | Drive to point + rotate | Simultaneous lateral+angular | Both settled |
| **turnTo** | Pure rotation | Quick heading changes | Heading settled |
| **follow** | Curved path | Pure Pursuit, smooth | Path complete + heading |

## Performance Tuning

### General Guidelines

1. **Start with conservative settings**
   - Lower PID gains
   - Longer timeouts
   - Larger exit ranges

2. **Gradually tighten tuning**
   - Increase PID gains until oscillation
   - Decrease timeout as confidence grows
   - Reduce exit ranges

3. **Test repeatedly**
   - Multiple runs
   - Different battery levels
   - Different field conditions

4. **Document working values**
   - Save tuned PID values
   - Record timeout values
   - Note exit condition parameters

## See Also

- [PID Controller](./pid.md) - Core control algorithm
- [ExitCondition](./exit-condition.md) - Motion termination logic
- [MotionHandler](./motion-handler.md) - Async execution
- [Configuration](../tutorials/2_configuration.md) - Default settings
- [Tutorials](../tutorials/index.md) - Practical examples
