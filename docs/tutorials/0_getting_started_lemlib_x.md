# Getting Started Tutorial

## Welcome to LemLib-X!

This tutorial walks you through setting up and running your first autonomous motion with LemLib-X.

## Prerequisites

- V5 Robot with VEX IQ Brain
- PROS installed (`pros cli`)
- Basic C++ knowledge
- LemLib-X template project

## Step 1: Project Setup

### Initialize PROS Project

```bash
# Create new PROS project with LemLib-X
pros fetch LemLib-X

# Navigate to project
cd LemLib-X

# Build to verify setup
pros build
```

### Verify Compilation

```bash
pros build --verbose

# Should complete without errors
```

## Step 2: Hardware Configuration

### Motor Setup

Edit `src/main.cpp` to define your motors:

```cpp
#include "main.h"

// Define motors for your drivetrain
lemlib::V5Motor front_left(1);      // Port 1
lemlib::V5Motor back_left(2);       // Port 2
lemlib::V5Motor front_right(3, true);  // Port 3, reversed
lemlib::V5Motor back_right(4, true);   // Port 4, reversed

// Create motor groups
std::vector<lemlib::Motor*> leftMotors = {&front_left, &back_left};
std::vector<lemlib::Motor*> rightMotors = {&front_right, &back_right};

lemlib::MotorGroup left_motors(leftMotors);
lemlib::MotorGroup right_motors(rightMotors);
```

### Sensor Setup

Add sensors to your configuration:

```cpp
// Tracking wheels (optional but recommended)
lemlib::ADIEncoder horizontal('A', 'B');  // Ports A, B
lemlib::ADIEncoder vertical('C', 'D');    // Ports C, D

// IMU
lemlib::V5InertialSensor imu(10);  // Port 10
```

### Create config.hpp

Create file `include/lemlib/config.hpp`:

```cpp
#pragma once

#include "lemlib/PID.hpp"
#include "lemlib/ExitCondition.hpp"
#include "units/Pose.hpp"
#include <functional>

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

## Step 3: Initialize Configuration

Complete `src/main.cpp` with configuration:

```cpp
// ============ CONFIG IMPLEMENTATION ============

// PID Controllers - start conservative
lemlib::PID angular_pid(0.7, 0.05, 0.05);
lemlib::PID lateral_pid(0.5, 0.02, 0.05);

// Physical parameters
const lemlib::Length track_width = 8_in;
const lemlib::Number drift_compensation = 0.5;
const lemlib::Number angular_slew = 360;
const lemlib::Number lateral_slew = 600;

// Odometry
lemlib::TrackingWheelOdom odom(horizontal, vertical, imu, track_width);

// Pose getter
const std::function<lemlib::units::Pose()> pose_getter = []() {
    return odom.getPose();
};

// Exit conditions
const lemlib::ExitConditionGroup<lemlib::Length> lateral_exit_conditions = {
    lemlib::ExitCondition<lemlib::Length>(2_in, 200_msec),
    lemlib::ExitCondition<lemlib::Length>(100_in, 5000_msec)
};

const lemlib::ExitConditionGroup<lemlib::Angle> angular_exit_conditions = {
    lemlib::ExitCondition<lemlib::Angle>(2_deg, 150_msec),
    lemlib::ExitCondition<lemlib::Angle>(360_deg, 5000_msec)
};

// Runtime setters
void setAngularPID(const lemlib::PID& pid) {
    angular_pid = pid;
}

void setLateralPID(const lemlib::PID& pid) {
    lateral_pid = pid;
}

// ============ INITIALIZATION ============
void initialize() {
    // Calibrate IMU
    imu.calibrate();
    while (imu.isCalibrating()) {
        pros::delay(50);
    }
    
    // Reset odometry
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    pros::lcd::print(0, "Ready!");
}

// ============ AUTONOMOUS ============
void autonomous() {
    // Reset starting position
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    // First motion: Move forward 24 inches
    lemlib::moveToPoint(lemlib::Point(24, 0), 5000_msec);
    
    // Code continues while motion runs in background
    pros::delay(100);
    
    // Wait for motion to complete
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    pros::lcd::print(0, "Forward motion complete!");
}
```

## Step 4: Compile and Test

### Build Project

```bash
pros build
```

### Upload to Robot

```bash
pros upload
```

### Test in Operator Control

```cpp
void opcontrol() {
    while (true) {
        // Tank drive control
        int leftPower = controller.get_analog(ANALOG_LEFT_Y);
        int rightPower = controller.get_analog(ANALOG_RIGHT_Y);
        
        left_motors.moveVoltage(leftPower * 120);
        right_motors.moveVoltage(rightPower * 120);
        
        // Update odometry
        odom.update();
        
        // Display position
        printf("Pos: (%.1f, %.1f, %.1f°)\n",
               odom.getPose().x.convert(inch),
               odom.getPose().y.convert(inch),
               odom.getPose().theta.convert(degree));
        
        pros::delay(10);
    }
}
```

## Step 5: Your First Autonomous Motion

### Simple Forward Movement

```cpp
void autonomous() {
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    // Move 24 inches forward
    lemlib::moveToPoint(lemlib::Point(24, 0), 5000_msec);
    
    // Wait for completion
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    std::cout << "Motion complete!" << std::endl;
}
```

### Motion Sequence

```cpp
void autonomous() {
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    // Move forward
    lemlib::moveToPoint(lemlib::Point(24, 0), 5000_msec);
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    // Turn right (face 90 degrees)
    lemlib::turnTo(90_deg, 2000_msec);
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    // Move right (strafe)
    lemlib::moveToPoint(lemlib::Point(24, 24), 5000_msec);
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    std::cout << "Autonomous complete!" << std::endl;
}
```

## Step 6: Tuning Your PID

### Initial Testing

Start with conservative gains:

```cpp
// In config
lemlib::PID lateral_pid(0.5, 0, 0);    // Only proportional
lemlib::PID angular_pid(0.7, 0, 0);
```

### Observe Robot Behavior

1. **Too slow?** → Increase kP
2. **Oscillating?** → Decrease kP or increase kD
3. **Overshooting?** → Decrease kP

### Iterative Tuning

```cpp
// Iteration 1: Increase response
lemlib::PID lateral_pid(0.8, 0, 0);

// Iteration 2: Add damping
lemlib::PID lateral_pid(0.8, 0, 0.1);

// Iteration 3: Add settling
lemlib::PID lateral_pid(0.8, 0.05, 0.1);
```

## Understanding Coordinate System

```
              90°   Forward
               ▲
               │
 Left 180° ────●──── Right 0°
               │
               ▼
              270°  Backward

Position (24, 0) = 24 inches to the right (East)
Position (0, 24) = 24 inches forward (North)
Position (24, 24) = 24 inches Northeast
```

## Common First Steps

### 1. Verify Motors Spin Correctly

```cpp
void opcontrol() {
    // Push right stick up - left and right motors should both spin forward
    if (controller.get_analog(ANALOG_RIGHT_Y) > 50) {
        left_motors.moveVoltage(8000);   // Should push robot forward
        right_motors.moveVoltage(8000);
    }
    
    // Check motor reversal flags if wrong direction
}
```

### 2. Verify Odometry

```cpp
void opcontrol() {
    // Drive forward 24 inches manually
    // Check if position changes from (0, 0) to (24, 0)
    
    auto pose = odom.getPose();
    printf("Pos: %.1f, %.1f\n", pose.x.convert(inch), pose.y.convert(inch));
}
```

### 3. Test Motion

```cpp
void autonomous() {
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    // Short test motion
    lemlib::moveToPoint(lemlib::Point(12, 0), 3000_msec);
    
    // Monitor and adjust
}
```

## Debugging Tips

### Print Odometry

```cpp
void printOdom() {
    auto pose = odom.getPose();
    printf("X=%.2f Y=%.2f θ=%.2f\n",
           pose.x.convert(inch),
           pose.y.convert(inch),
           pose.theta.convert(degree));
}
```

### Check Motor Voltage

```cpp
printf("Left voltage: %f\n", left_motors.getVoltage());
printf("Right voltage: %f\n", right_motors.getVoltage());
```

### Monitor Motion Status

```cpp
if (lemlib::motion_handler::isMoving()) {
    std::cout << "Motion in progress" << std::endl;
}
```

## Next Steps

1. **Read [PID Tuning Guide](./api/pid.md)** - Detailed tuning strategies
2. **Explore [Motion Algorithms](./api/motions.md)** - All available motions
3. **Learn [Advanced Configuration](./api/configuration.md)** - Optimize per robot
4. **Study [Path Following](./tutorials/7_pure_pursuit.md)** - Curved paths
5. **Master [Motion Sequences](./tutorials/8_motion_chaining.md)** - Complex autonomy

## Troubleshooting

### Motors Don't Move

1. Check port numbers (1-21)
2. Verify motor wiring
3. Test in opcontrol first
4. Check for reversed flags

### Robot Goes Wrong Direction

1. Set reversed flag: `V5Motor(port, true)`
2. Verify chassis is symmetric

### Motion Doesn't End

1. Check timeout value
2. Verify exit conditions
3. Tune PID gains
4. Check for stuck mechanisms

### Odometry Wrong

1. Calibrate IMU thoroughly
2. Check tracking wheel placement
3. Verify encoder connections
4. Test known distances

## Support

- **Documentation**: See [API Reference](./api/index.md)
- **Examples**: Check `examples/` folder
- **Issues**: Report on GitHub
- **Community**: LemLib Discord server

Congratulations! You now have a working LemLib-X autonomous system! 🎉
