# LemLib-X Architecture Overview

## Introduction

LemLib-X is a high-performance robotics library designed for VEX V5 robots. It provides autonomous motion control, odometry tracking, and a comprehensive motion system built on top of the PROS (Purdue Robotics Operating System) kernel.

## Project Structure

### Core Components

```
lemlib/
├── motions/           # Motion algorithms
│   ├── follow.hpp     # Path following (Pure Pursuit)
│   ├── moveToPoint.hpp # Drive to a point
│   ├── moveToPose.hpp  # Drive to a specific position and angle
│   └── turnTo.hpp      # Rotate to a target angle
├── tracking/          # Odometry and position tracking
│   └── TrackingWheelOdom.hpp
├── PID.hpp           # PID controller implementation
├── MotionHandler.hpp # Motion task scheduler
├── ExitCondition.hpp # Motion termination conditions
├── config.hpp        # Global configuration and defaults
├── Timer.hpp         # Timing utilities
├── util.hpp          # Utility functions
└── Page/PageManager  # UI pages for tuning and debugging
```

### Hardware Abstraction Layer

```
hardware/
├── Motor/
│   ├── Motor.hpp       # Single motor wrapper
│   └── MotorGroup.hpp  # Group of motors (left/right sides)
├── Encoder/
│   ├── Encoder.hpp         # Generic encoder interface
│   ├── ADIEncoder.hpp       # Analog encoder
│   └── V5RotationSensor.hpp # V5 built-in rotation sensor
└── IMU/
    ├── IMU.hpp              # Generic IMU interface
    └── V5InertialSensor.hpp # V5 inertial measurement unit
```

### Utility Components

```
units/
├── Angle.hpp          # Angle measurements (degrees/radians)
├── Length.hpp         # Distance measurements (inches/cm)
├── Pose.hpp           # 2D position with angle
├── Vector2D.hpp       # 2D vector operations
├── Vector3D.hpp       # 3D vector operations
└── Temperature.hpp    # Temperature measurements
```

## System Architecture

### 1. Motion System Architecture

```
┌─────────────────────────────────────────────┐
│         User Motion Request                 │
│   (moveToPoint, follow, turnTo, etc)        │
└────────────────┬────────────────────────────┘
                 │
                 ▼
        ┌─────────────────────┐
        │  MotionHandler      │
        │  (Task scheduler)   │
        └────────┬────────────┘
                 │
        ┌────────▼──────────┐
        │ Motion Algorithm  │ (runs in background task)
        │  (follow, etc)    │
        └────────┬──────────┘
                 │
        ┌────────▼─────────────────┐
        │  PID Controllers         │
        │  - Lateral              │
        │  - Angular              │
        └────────┬─────────────────┘
                 │
        ┌────────▼──────────────────┐
        │  Exit Conditions         │
        │  (timeout, settled, etc) │
        └────────┬──────────────────┘
                 │
        ┌────────▼──────────────┐
        │  Odometry Tracking    │
        │  (position update)    │
        └────────┬──────────────┘
                 │
        ┌────────▼─────────────────────┐
        │  Motor Control Layer         │
        │  - MotorGroup (L/R)          │
        │  - Voltage/Velocity commands │
        └────────┬─────────────────────┘
                 │
                 ▼
        ┌─────────────────────┐
        │  V5 Motors & IMU    │
        │  (physical hardware)│
        └─────────────────────┘
```

### 2. Odometry System

The odometry system continuously tracks the robot's position using:

- **Tracking Wheels**: Optionally positioned tracking wheels for high-precision odometry
- **IMU Integration**: Inertial measurement unit for heading correction
- **Motor Encoders**: Motor encoder feedback for fallback positioning

Position is updated at regular intervals via the `TrackingWheelOdom` class.

### 3. Control Flow

```
Configuration (config.hpp)
    ↓
─── Setup ───────────────────────
    └─ Define motor groups (left, right)
    └─ Set PID gains (angular, lateral)
    └─ Configure exit conditions
    └─ Setup odometry (pose getter)
    
─── Runtime ──────────────────────
    ├─ Motion Request
    │  └─ lemlib::moveToPoint(target)
    │     lemlib::motion_handler::move(fn)
    │
    ├─ Background Motion Task
    │  ├─ Update odometry
    │  ├─ Calculate PID outputs
    │  ├─ Check exit conditions
    │  └─ Apply motor commands
    │
    └─ Motion Complete
       └─ Calls completion handler
```

## Key Design Principles

### 1. Non-Blocking Architecture
- Motion algorithms run in background tasks
- User code can continue executing while motion runs
- Motion handler enforces serialization of motions

### 2. Modular Component Design
- Each component (PID, Odometry, MotionHandler) is independent
- Components can be swapped or modified without affecting others
- Hardware layer abstractions allow different sensor configurations

### 3. Configuration-Driven Defaults
- Global configuration centralized in `config.hpp`
- Override defaults at motion call time with params
- Simplifies function signatures with common parameters

### 4. Type-Safe Units
- Uses template-based unit system for safety
- Prevents mixing inches with cm or degrees with radians
- Compile-time checking of unit compatibility

### 5. Real-Time Performance
- Minimal memory allocations in control loops
- Fixed-size exit condition groups
- Efficient PID implementation with minimal branching

## Data Flow Overview

### From Sensors to Motion Output

```
Sensors (Motor Encoders, IMU, Tracking Wheels)
    ↓
Odometry Update (TrackingWheelOdom)
    ↓
Current Pose (pose_getter callback)
    ↓
Error Calculation
    ├─ Lateral Error: distance to target
    └─ Angular Error: heading to target
    ↓
PID Controllers
    ├─ Lateral PID → forward/backward velocity
    └─ Angular PID → rotational velocity
    ↓
Exit Condition Check
    ├─ Has timeout elapsed?
    └─ Is error within acceptable range for long enough?
    ↓
Motor Commands (velocity or voltage)
    ↓
Actuators (V5 Motors)
    ↓
Robot Movement
```

## Configuration System

The `config.hpp` file defines global defaults that all motions use:

```cpp
// Motor groups
extern lemlib::MotorGroup left_motors;
extern lemlib::MotorGroup right_motors;

// PID controllers
extern lemlib::PID angular_pid;  // Rotation control
extern lemlib::PID lateral_pid;  // Linear movement control

// Odometry
extern const std::function<units::Pose()> pose_getter;

// Physical parameters
extern const Length track_width;  // Distance between left/right motors
extern const Number drift_compensation;

// Slew rate (acceleration limiting)
extern const Number angular_slew;    // Max angular acceleration
extern const Number lateral_slew;    // Max linear acceleration
```

These can be updated at runtime with `setAngularPID()` and `setLateralPID()`.

## Threading Model

LemLib-X uses PROS tasks for concurrent execution:

- **Main Task**: Your autonomous/driver code
- **Motion Task**: Background task running motion algorithms
- **Sensor Tasks**: PROS handles these (motor feedback, IMU)

The MotionHandler ensures only one motion runs at a time via task notification.

## Performance Considerations

1. **Update Loop Frequency**: Motion algorithms typically run at 100 Hz (10 ms cycles)
2. **PID Tuning**: Start with low gains, increase until oscillation appears
3. **Exit Conditions**: Balance responsiveness vs. stability
4. **Slew Rate**: Limits acceleration to prevent wheel slip
5. **Tracking Wheel Placement**: Optimal perpendicular arrangement improves accuracy

## Integration Points

### Customize Motion Behavior

- Override `pose_getter` to use custom odometry
- Replace PID controllers for different tuning profiles
- Implement custom exit conditions via `ExitConditionGroup`
- Extend MotionHandler for specialized motion sequences

### Add Custom Motions

Create custom motion algorithms that follow the MotionHandler pattern:

```cpp
void customMotion() {
    std::uint32_t prevTime = pros::millis();
    while (!pros::Task::notify_take(true, 0)) {
        // Your motion logic here
        pros::Task::delay_until(&prevTime, 10);
    }
}

// Run it:
lemlib::motion_handler::move(customMotion);
```

## Summary

LemLib-X provides a complete motion control system with:
- **Flexible Motion Algorithms**: Pure pursuit, point-to-point, angle control
- **Robust Odometry**: Multi-sensor tracking with drift compensation
- **Task-Based Concurrency**: Non-blocking asynchronous motion execution
- **Type-Safe Units**: Compile-time unit verification
- **Configurable Defaults**: Easily customizable for different robots

The architecture prioritizes performance, safety, and ease of use for competitive robotics development.
