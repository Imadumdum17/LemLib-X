# Configuration Guide

## Overview

LemLib-X uses a centralized configuration system (`config.hpp`) to define robot-specific parameters, motor groups, PID controllers, and default settings. This allows easy customization without modifying motion algorithm code.

## Configuration File Structure

### config.hpp Location

```
include/
└── lemlib/
    └── config.hpp
```

### File Template

```cpp
#pragma once

#include "ExitCondition.hpp"
#include "PID.hpp"
#include "hardware/Motor/MotorGroup.hpp"
#include "units/Pose.hpp"
#include <functional>

// Motor Groups
// PID Controllers
// Pose Getter Function
// Physical Parameters
// Exit Conditions
// Slew Rate Limits

// Runtime Configuration Functions
void setAngularPID(const lemlib::PID& pid);
void setLateralPID(const lemlib::PID& pid);
```

## Required Configurations

### 1. Motor Groups

Define the motors driving the robot.

```cpp
// Left side motors
extern lemlib::MotorGroup left_motors;

// Right side motors
extern lemlib::MotorGroup right_motors;
```

**Implementation in main.cpp:**

```cpp
#include "config.hpp"

// Create individual motors
lemlib::V5Motor front_left(1);
lemlib::V5Motor back_left(2);
lemlib::V5Motor front_right(3, true);  // Reversed
lemlib::V5Motor back_right(4, true);   // Reversed

// Create motor groups
std::vector<lemlib::Motor*> left = { &front_left, &back_left };
std::vector<lemlib::Motor*> right = { &front_right, &back_right };

lemlib::MotorGroup left_motors(left);
lemlib::MotorGroup right_motors(right);
```

### 2. PID Controllers

Define proportional-integral-derivative gains.

```cpp
// Lateral (forward/backward) PID
extern lemlib::PID lateral_pid;

// Angular (rotational) PID
extern lemlib::PID angular_pid;
```

**Implementation in main.cpp:**

```cpp
// Lateral PID - controls driving forward/backward
// Start with conservative gains
lemlib::PID lateral_pid(0.8, 0.05, 0.1, 50, true);

// Angular PID - controls rotation
lemlib::PID angular_pid(1.0, 0.1, 0.15, 100, true);
```

**Tuning Guide:**
- Start with low gains (0.5, 0.05, 0.05)
- Increase kP until oscillation appears
- Add kD for damping
- Add small kI if steady-state error remains

### 3. Pose Getter Function

Callback that returns current robot position.

```cpp
extern const std::function<units::Pose()> pose_getter;
```

**Implementation in main.cpp:**

```cpp
// Create odometry system
lemlib::TrackingWheelOdom odom(/* params */);

// Define pose getter
const std::function<units::Pose()> pose_getter = []() {
    return odom.getPose();
};

// This is called by motion algorithms:
// auto pose = pose_getter();  // Returns robot's current position
```

### 4. Physical Parameters

Robot-specific dimensions and characteristics.

```cpp
// Distance between left and right motor wheels (inches)
extern const Length track_width;

// Compensation factor for heading drift (0.0 to 1.0)
extern const Number drift_compensation;

// Maximum acceleration rates
extern const Number angular_slew;    // degrees/sec²
extern const Number lateral_slew;    // inches/sec²
```

**Implementation in main.cpp:**

```cpp
#include "config.hpp"

// Tank drive measurement
const Length track_width = 8_in;  // 8 inch wide drivetrain

// Use default drift compensation
const Number drift_compensation = 0.5;

// Slew rates limit acceleration to prevent wheel slip
const Number angular_slew = 360;   // degrees/sec²
const Number lateral_slew = 600;   // inches/sec²
```

### 5. Exit Conditions

Define when motions are considered complete.

```cpp
extern const lemlib::ExitConditionGroup<Angle> angular_exit_conditions;
extern const lemlib::ExitConditionGroup<Length> lateral_exit_conditions;
```

**Implementation in main.cpp:**

```cpp
const lemlib::ExitConditionGroup<Length> lateral_exit_conditions = {
    // Primary: Settled within 2 inches for 200ms
    lemlib::ExitCondition<Length>(2_in, 200_msec),
    
    // Fallback: Settled within 5 inches for 500ms
    lemlib::ExitCondition<Length>(5_in, 500_msec),
    
    // Safety: Timeout after 5 seconds
    lemlib::ExitCondition<Length>(100_in, 5000_msec)
};

const lemlib::ExitConditionGroup<Angle> angular_exit_conditions = {
    // Primary: Settled within 2° for 150ms
    lemlib::ExitCondition<Angle>(2_deg, 150_msec),
    
    // Fallback: Settled within 5° for 300ms
    lemlib::ExitCondition<Angle>(5_deg, 300_msec),
    
    // Safety: Timeout after 5 seconds
    lemlib::ExitCondition<Angle>(360_deg, 5000_msec)
};
```

## Runtime Configuration Functions

### setAngularPID

Update angular PID gains without recompiling.

```cpp
void setAngularPID(const lemlib::PID& pid);
```

**Usage:**

```cpp
void autonomous() {
    // Change tuning at runtime
    lemlib::Gains newGains = {1.2, 0.1, 0.2};
    setAngularPID(lemlib::PID(newGains));
    
    // Motion algorithms now use new gains
    moveToPoint(24, 24);
}
```

### setLateralPID

Update lateral PID gains without recompiling.

```cpp
void setLateralPID(const lemlib::PID& pid);
```

**Usage:**

```cpp
void autonomous() {
    // Adjust lateral tuning
    lemlib::Gains lateralGains = {0.9, 0.05, 0.12};
    setLateralPID(lemlib::PID(lateralGains));
    
    // All lateral motions use new gains
    moveToPoint(Point(0, 24));
}
```

## Complete Configuration Example

### Minimal Configuration

```cpp
// File: include/lemlib/config.hpp
#pragma once

#include "lemlib/PID.hpp"
#include "lemlib/ExitCondition.hpp"
#include "lemlib/hardware/Motor/MotorGroup.hpp"

// Forward declarations
extern lemlib::MotorGroup left_motors;
extern lemlib::MotorGroup right_motors;
extern lemlib::PID angular_pid;
extern lemlib::PID lateral_pid;
extern const std::function<lemlib::units::Pose()> pose_getter;
extern const lemlib::Length track_width;
extern const lemlib::Number drift_compensation;
extern const lemlib::Number angular_slew;
extern const lemlib::Number lateral_slew;
extern const lemlib::ExitConditionGroup<lemlib::Angle> angular_exit_conditions;
extern const lemlib::ExitConditionGroup<lemlib::Length> lateral_exit_conditions;

void setAngularPID(const lemlib::PID& pid);
void setLateralPID(const lemlib::PID& pid);
```

### Implementation Example

```cpp
// File: src/main.cpp
#include "main.h"
#include "config.hpp"
#include "lemlib/TrackingWheelOdom.hpp"

// ================== MOTORS ==================
lemlib::V5Motor front_left(1);
lemlib::V5Motor back_left(2);
lemlib::V5Motor front_right(3, true);
lemlib::V5Motor back_right(4, true);

std::vector<lemlib::Motor*> leftMotors = { &front_left, &back_left };
std::vector<lemlib::Motor*> rightMotors = { &front_right, &back_right };

lemlib::MotorGroup left_motors(leftMotors);
lemlib::MotorGroup right_motors(rightMotors);

// ================== SENSORS ==================
lemlib::ADIEncoder horizontal('A', 'B');
lemlib::ADIEncoder vertical('C', 'D');
lemlib::V5InertialSensor imu(10);

// ================== ODOMETRY ==================
lemlib::TrackingWheelOdom odom(horizontal, vertical, imu, 8_in);

// ================== CONFIG VARIABLES =========
lemlib::PID angular_pid(1.0, 0.1, 0.15, 100, true);
lemlib::PID lateral_pid(0.8, 0.05, 0.1, 50, true);

const std::function<lemlib::units::Pose()> pose_getter = 
    []() { return odom.getPose(); };

const lemlib::Length track_width = 8_in;
const lemlib::Number drift_compensation = 0.5;
const lemlib::Number angular_slew = 360;
const lemlib::Number lateral_slew = 600;

const lemlib::ExitConditionGroup<lemlib::Length> lateral_exit_conditions = {
    lemlib::ExitCondition<lemlib::Length>(2_in, 200_msec),
    lemlib::ExitCondition<lemlib::Length>(5_in, 500_msec),
    lemlib::ExitCondition<lemlib::Length>(100_in, 5000_msec)
};

const lemlib::ExitConditionGroup<lemlib::Angle> angular_exit_conditions = {
    lemlib::ExitCondition<lemlib::Angle>(2_deg, 150_msec),
    lemlib::ExitCondition<lemlib::Angle>(5_deg, 300_msec),
    lemlib::ExitCondition<lemlib::Angle>(360_deg, 5000_msec)
};

// ================== RUNTIME SETTERS =========
void setAngularPID(const lemlib::PID& pid) {
    angular_pid = pid;
}

void setLateralPID(const lemlib::PID& pid) {
    lateral_pid = pid;
}

// ================== INITIALIZATION =========
void initialize() {
    // Calibrate IMU
    imu.calibrate();
    while (imu.isCalibrating()) {
        pros::delay(50);
    }
    
    // Reset odometry
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    std::cout << "Configuration loaded successfully!" << std::endl;
}

// ================== AUTONOMOUS ===============
void autonomous() {
    // System is ready to use
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    lemlib::moveToPoint(Point(24, 0), 5000_msec);
}
```

## Parameter Tuning Workflow

### Step 1: Initial Setup

```cpp
// Start conservative
lemlib::PID lateral_pid(0.5, 0, 0);    // Pure P
lemlib::PID angular_pid(0.7, 0, 0);
```

### Step 2: Test and Observe

```cpp
// In autonomous
void autonomous() {
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    // Try simple motion
    lemlib::moveToPoint(Point(24, 0), 5000_msec);
    
    // Observe behavior:
    // - Too slow: Increase kP
    // - Oscillates: Decrease kP
    // - Overshoots: Decrease kP or increase kD
}
```

### Step 3: Tune Systematically

```cpp
// Iteration 1: Increase proportional
lemlib::PID lateral_pid(0.8, 0, 0);    // Better response

// Iteration 2: Add damping if oscillates
lemlib::PID lateral_pid(0.8, 0, 0.1);  // Add D term

// Iteration 3: Add integral if steady-state error
lemlib::PID lateral_pid(0.8, 0.05, 0.1);  // Add small I
```

### Step 4: Finalize

```cpp
// Document working tuning
const char* TUNING_VERSION = "v1.2";

lemlib::PID lateral_pid(0.85, 0.05, 0.12);  // Tested & verified
lemlib::PID angular_pid(1.1, 0.08, 0.18);
```

## Common Configuration Patterns

### Light Robot (Fast)

```cpp
const lemlib::Number lateral_slew = 900;   // Aggressive
const lemlib::Number angular_slew = 450;

lemlib::PID lateral_pid(1.2, 0.08, 0.15);
lemlib::PID angular_pid(1.4, 0.12, 0.2);
```

### Medium Robot (Standard)

```cpp
const lemlib::Number lateral_slew = 600;
const lemlib::Number angular_slew = 360;

lemlib::PID lateral_pid(0.8, 0.05, 0.1);
lemlib::PID angular_pid(1.0, 0.1, 0.15);
```

### Heavy Robot (Slow/Strong)

```cpp
const lemlib::Number lateral_slew = 300;   // Conservative
const lemlib::Number angular_slew = 180;

lemlib::PID lateral_pid(0.6, 0.03, 0.08);
lemlib::PID angular_pid(0.8, 0.05, 0.12);
```

## Troubleshooting Configuration

### Issue: Compile Error - "Undefined reference to lateral_pid"

**Cause:** Didn't implement extern declarations  
**Solution:** Add implementation in main.cpp:
```cpp
lemlib::PID lateral_pid(0.8, 0.05, 0.1);
```

### Issue: Motors Spin Wrong Direction

**Cause:** Motor reversal flags incorrect  
**Solution:** Check motor constructor 'reversed' parameter:
```cpp
lemlib::V5Motor right_motor(3, true);  // Add true to reverse
```

### Issue: Motions Don't Use New PID Gains

**Cause:** Runtime setter not called  
**Solution:** Call setter before motion:
```cpp
setLateralPID(lemlib::PID(newGains));
moveToPoint(target);
```

## Best Practices

1. **Document tuning values** with date and conditions
2. **Version your configurations** as you improve them
3. **Test on actual field** - battery and friction affect behavior
4. **Keep backups** if tuning worked previously
5. **Update comments** when changing parameters
6. **Use consistent units** (inches, degrees)
7. **Test motion sequences** - verify timeout values
8. **Monitor feedback** for any hardware changes

## See Also

- [PID Controller](./api/pid.md) - Tuning guide
- [Hardware](./api/hardware.md) - Motor/sensor setup
- [Motion Algorithms](./api/motions.md) - Usage examples
- [Tutorials](./tutorials/) - Practical configuration guide
