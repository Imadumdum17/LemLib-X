# LemLib-X API Reference

## Welcome to the LemLib-X API Documentation

Complete reference documentation for the LemLib-X robotics library. LemLib-X is a high-performance autonomous motion library for VEX V5 robots, featuring precise odometry, PID control, and path following capabilities.

## Core Components

### Motion Control

- **[Motion Algorithms](./motions.md)**
  - `moveToPoint()` - Drive to a target location
  - `moveToPose()` - Drive to location while facing direction
  - `turnTo()` - Rotate to target heading
  - `follow()` - Follow pre-generated path (Pure Pursuit)

- **[PID Controller](./pid.md)**
  - Proportional-Integral-Derivative control
  - Real-time gain adjustment
  - Tuning guide and parameter reference

- **[Motion Handler](./motion-handler.md)**
  - Asynchronous motion task scheduler
  - Non-blocking motion execution
  - Motion cancellation support

- **[Exit Conditions](./exit-condition.md)**
  - Motion completion detection
  - Timeout management
  - Custom settling criteria

### Positioning & Tracking

- **[Odometry & Tracking](./odometry.md)**
  - Position and heading tracking
  - Tracking wheel integration
  - IMU fusion and drift compensation
  - Multi-sensor odometry

### Hardware Integration

- **[Hardware Abstraction](./hardware.md)**
  - Motor control (V5Motor, MotorGroup)
  - Encoders (ADIEncoder, V5RotationSensor)
  - IMU sensors (V5InertialSensor)
  - Sensor configuration

### Configuration

- **[Configuration Guide](./configuration.md)**
  - Setting up motor groups
  - Tuning PID parameters
  - Configuring exit conditions
  - Runtime parameter updates

### User Interface

- **[Page System](./pages.md)**
  - Real-time tuning UI
  - PID adjustment interface
  - Telemetry display
  - Custom page creation

### Utilities

- **[Utilities & Helpers](./utilities.md)**
  - Timer management
  - Vector mathematics (2D/3D)
  - Unit system (Angle, Length, Time)
  - Pose calculations
  - Math functions

## Quick Navigation

```{toctree}
:maxdepth: 2
:caption: Motion Control

./motions
./pid
./motion-handler
./exit-condition
```

```{toctree}
:maxdepth: 2
:caption: Positioning

./odometry
./hardware
```

```{toctree}
:maxdepth: 2
:caption: Setup & Tuning

./configuration
./pages
./utilities
```

## Getting Started

1. **New to LemLib-X?** → Start with [Getting Started Tutorial](../tutorials/0_getting_started_lemlib_x.md)
2. **Setting up hardware?** → Read [Hardware Integration](./hardware.md)
3. **Tuning motions?** → See [PID Controller](./pid.md)
4. **Creating custom motions?** → Check [Motion Handler](./motion-handler.md)

## Quick Reference

### Motion Commands

```cpp
// Forward movement
lemlib::moveToPoint(Point(24, 0), 5000_msec);

// Turn in place
lemlib::turnTo(90_deg, 2000_msec);

// Move to location + heading
lemlib::moveToPose(Pose(24, 24, 90_deg), 5000_msec);

// Follow path
lemlib::follow(pathAsset, 8_in, 10000_msec);
```

### Coordinate System

```
                90°
                ▲
     180° ◄── Origin ──► 0°
                ▼
               270°

(X, Y, Theta) format
X: Right (East)
Y: Forward (North)
Theta: Counter-clockwise rotation
```

### Configuration

```cpp
// Motor groups
extern lemlib::MotorGroup left_motors;
extern lemlib::MotorGroup right_motors;

// PID controllers
extern lemlib::PID lateral_pid;
extern lemlib::PID angular_pid;

// Physical parameters
extern const Length track_width;
extern const Number drift_compensation;
```

## Architecture Overview

```
Motion Algorithms (moveToPoint, follow, etc)
         ↓
Motion Handler (Async task management)
         ↓
PID Controllers (Lateral, Angular)
         ↓
 Exit Conditions (Settling detection)
         ↓
Odometry System (Position tracking)
         ↓
Hardware Layer (Motors, Sensors, IMU)
         ↓
V5 Robot & Sensors
```

## Common Tasks

### Setting Up Your Robot

1. [Configure hardware](./hardware.md) - Motors, encoders, IMU
2. [Create motor groups](./configuration.md) - Define drivetrain
3. [Initialize odometry](./odometry.md) - Position tracking
4. [Set PID values](./pid.md) - Tune control loops

### Running Autonomous

1. Reset odometry: `odom.setPose(Pose(0, 0, 0_deg))`
2. Schedule motion: `lemlib::moveToPoint(target, timeout)`
3. Wait for completion: Loop while `isMoving()`
4. Check result: Use odometry to verify position

### Tuning Performance

1. [Adjust PID gains](./pid.md) - Response tuning
2. [Edit exit conditions](./exit-condition.md) - Settling criteria
3. [Use Page System](./pages.md) - Real-time UI
4. [Test motions](../tutorials/0_getting_started_lemlib_x.md) - Verify behavior

## Support & Community

- **Documentation Issues**: See [Support](../support.md)
- **API Questions**: Check relevant section
- **Bug Reports**: Open GitHub issue
- **Discussions**: LemLib Discord/Community forums

## Related Documentation

- 📚 [System Architecture](../architecture.md) - High-level overview
- 🚀 [Getting Started](../tutorials/0_getting_started_lemlib_x.md) - First steps
- 🔧 [Configuration](./configuration.md) - Robot setup
- 📖 [Tutorials](../tutorials/) - Practical examples
- 💬 [Support](../support.md) - Getting help
