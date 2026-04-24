# Hardware Abstraction Layer Documentation

## Overview

LemLib-X provides abstraction layers for common robotics hardware, allowing flexible configuration and easy swapping of components. This layer handles communication with V5 sensors and actuators.

## Architecture

```
┌─────────────────────────────────────────┐
│     High-Level Motion Algorithms        │
│   (moveToPoint, follow, etc.)           │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│    Hardware Abstraction Interfaces      │
│  (Motor, Encoder, IMU abstract classes) │
└──────────────┬──────────────────────────┘
               │
┌──────────────▼──────────────────────────┐
│    Concrete Implementations             │
│  - V5Motor, ADIEncoder, V5InertialSensor│
│  - Custom implementations possible      │
└──────────────┬──────────────────────────┘
               │
               ▼
        V5 Brain & Sensors
```

## Motor Abstraction

### Motor Base Class

Represents a single motor.

```cpp
class Motor {
    public:
        virtual void moveVoltage(float voltage) = 0;
        virtual void moveVelocity(int velocity) = 0;
        virtual float getVoltage() const = 0;
        virtual int getVelocity() const = 0;
        virtual float getPosition() const = 0;
        virtual void reset() = 0;
};
```

### V5Motor Implementation

Controls a single V5 motor.

```cpp
// Constructor
V5Motor(uint8_t port, bool reversed = false, gearset gear = E_MOTOR_GEARSET_18);

// Parameters:
// - port: 1-21 (V5 motor ports)
// - reversed: Whether to reverse motor direction
// - gear: E_MOTOR_GEARSET_18 (200RPM), 
//         E_MOTOR_GEARSET_06 (600RPM), or
//         E_MOTOR_GEARSET_36 (100RPM)
```

**Usage:**

```cpp
// Create single V5 motor
lemlib::V5Motor leftMotor(1);   // Port 1, not reversed
lemlib::V5Motor rightMotor(2, true);  // Port 2, reversed

// Move with voltage (-12000 to +12000)
leftMotor.moveVoltage(12000);   // Full forward

// Move with velocity (rpm)
rightMotor.moveVelocity(600);   // 600 RPM

// Get feedback
printf("Position: %f\n", leftMotor.getPosition());
printf("Velocity: %d\n", leftMotor.getVelocity());
```

### MotorGroup

Groups multiple motors together (e.g., left side of tank drive).

```cpp
class MotorGroup {
    public:
        MotorGroup(std::vector<Motor*> motors);
        
        void moveVoltage(float voltage);   // All motors, same voltage
        void moveVelocity(int velocity);   // All motors, same velocity
        float getVoltage() const;          // Average voltage
        int getVelocity() const;           // Average velocity
        float getPosition() const;         // Average position
        void reset();                      // Reset all encoders
};
```

**Usage:**

```cpp
// Create individual motors
lemlib::V5Motor front_left(1);
lemlib::V5Motor back_left(2);
lemlib::V5Motor front_right(3, true);
lemlib::V5Motor back_right(4, true);

// Group left side motors
std::vector<lemlib::Motor*> left_motors = {
    &front_left,
    &back_left
};
lemlib::MotorGroup left_group(left_motors);

// Group right side motors
std::vector<lemlib::Motor*> right_motors = {
    &front_right,
    &back_right
};
lemlib::MotorGroup right_group(right_motors);

// In config.hpp
extern lemlib::MotorGroup left_motors = left_group;
extern lemlib::MotorGroup right_motors = right_group;

// Usage in motion
left_motors.moveVoltage(8000);   // Full power
std::cout << "Left speed: " << left_motors.getVelocity() << std::endl;
```

## Encoder Abstraction

### Encoder Base Class

Represents feedback from rotational sensors.

```cpp
class Encoder {
    public:
        virtual float getPosition() = 0;  // Revolutions or degrees
        virtual void reset() = 0;
};
```

### ADI Encoder (3-Wire/Analog)

For older analog encoders (e.g., legacy encoders).

```cpp
ADIEncoder(uint8_t portTop, uint8_t portBottom);

// Parameters:
// - portTop: ADI port for top pin (A, B, C, D, E, F, G, H)
// - portBottom: ADI port for bottom pin
```

**Usage:**

```cpp
// Create ADI encoder on ports A and B
lemlib::ADIEncoder horizontalEncoder('A', 'B');

// Get position in revolutions
float revolutions = horizontalEncoder.getPosition();

// Reset position
horizontalEncoder.reset();

// Typical tracking wheel setup
lemlib::ADIEncoder trackingEnc('C', 'D');
float distance_traveled = trackingEnc.getPosition() * WHEEL_CIRCUMFERENCE;
```

### V5RotationSensor

Built-in V5 motor encoder.

```cpp
V5RotationSensor(uint8_t port);

// Parameters:
// - port: 1-21 (sensor port on V5 Brain)
```

**Usage:**

```cpp
// Create rotation sensor on port 11
lemlib::V5RotationSensor motorFeedback(11);

// Get position in degrees
int degrees = motorFeedback.getPosition();

// Reset
motorFeedback.reset();

// Use for motor encoder feedback
// (alternative to motor internal encoder)
```

## IMU Abstraction

### IMU Base Class

Inertial Measurement Unit providing heading and acceleration data.

```cpp
class IMU {
    public:
        virtual float getHeading() = 0;    // Yaw angle in degrees
        virtual void reset() = 0;
        virtual void calibrate() = 0;
        virtual bool isCalibrating() = 0;
};
```

### V5InertialSensor

Built-in V5 IMU.

```cpp
V5InertialSensor(uint8_t port);

// Parameters:
// - port: 1-21 (sensor port on V5 Brain)
```

**Usage:**

```cpp
// Create IMU on port 10
lemlib::V5InertialSensor imu(10);

// Calibration (required at startup)
imu.calibrate();

// Wait for calibration to complete
while (imu.isCalibrating()) {
    pros::delay(50);  // Do NOT rotate robot
}

std::cout << "IMU ready!" << std::endl;

// Get heading in degrees
float heading = imu.getHeading();

// Reset heading to 0
imu.reset();

// In autonomous loop
if (imu.getHeading() > 90) {
    // Robot has rotated past 90 degrees
}
```

## Complete System Example

### Hardware Setup

```cpp
// File: src/main.cpp

#include "main.h"
#include "lemlib/api.hpp"

// Motors
lemlib::V5Motor frontLeft(1);
lemlib::V5Motor backLeft(2);
lemlib::V5Motor frontRight(3, true);  // Reversed
lemlib::V5Motor backRight(4, true);   // Reversed

// Sensors
lemlib::ADIEncoder horizontalEnc('A', 'B');
lemlib::ADIEncoder verticalEnc('C', 'D');
lemlib::V5InertialSensor imu(10);

// Motor Groups
std::vector<lemlib::Motor*> leftMotors = {
    &frontLeft,
    &backLeft
};
std::vector<lemlib::Motor*> rightMotors = {
    &frontRight,
    &backRight
};

lemlib::MotorGroup left_motors(leftMotors);
lemlib::MotorGroup right_motors(rightMotors);

// Odometry
lemlib::TrackingWheelOdom odom(
    horizontalEnc,
    verticalEnc,
    imu,
    8_in  // Track width
);

void initialize() {
    // Calibrate IMU
    imu.calibrate();
    while (imu.isCalibrating()) {
        pros::delay(50);
    }
    
    // Reset encoders
    horizontalEnc.reset();
    verticalEnc.reset();
    
    // Reset position
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    std::cout << "Initialization complete" << std::endl;
}
```

### Usage Flow

```cpp
void autonomous() {
    // System is now ready:
    // - Motors configured
    // - Encoders calibrated
    // - IMU calibrated
    // - Odometry ready
    
    // Use in motion
    lemlib::moveToPoint(Point(24, 0), 5000_msec);
    
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
}
```

## Port Numbering

### V5 Motor/Sensor Ports

```
Front of Robot

Ports 1-21 available on V5 Brain
Standard Tank Drive Setup:
- Port 1: Front Left Motor
- Port 2: Back Left Motor
- Port 3: Front Right Motor
- Port 4: Back Right Motor

Sensors:
- Port 10: IMU
- Port 11: Horizontal Tracking (optional)
- Port 12: Vertical Tracking (optional)
```

### ADI Ports (3-Wire)

Connected via ADI Expander Plugin or Legacy UART Port:

```
Ports available: A, B, C, D, E, F, G, H

Typical Tracking Configuration:
- Ports A, B: Horizontal Encoder
- Ports C, D: Vertical Encoder
```

## Reversing Motors

Handle drivetrain assymetries:

```cpp
// Create right-side motor reversed
lemlib::V5Motor rightMotor(3, true);  // true = reversed

// Now when left gets +100V, right also gets +100V (but spins opposite)
// This makes both motors push the robot forward
```

## Gearset Selection

Choose appropriate motor gear ratio:

```cpp
// Constructor takes optional gearset parameter
V5Motor(port, reversed, gearset);

// Available gearsets:
// E_MOTOR_GEARSET_36 (100 RPM)  - Torque (slow, strong)
// E_MOTOR_GEARSET_18 (200 RPM)  - Balanced (default)
// E_MOTOR_GEARSET_06 (600 RPM)  - Speed (fast, weak)
```

**Tank Drive Recommendation:** Use 200 RPM (balanced) for most drivetrains.

## Common Configurations

### 4-Motor Tank Drive

```cpp
// Front left, Back left, Front right, Back right
std::vector<lemlib::Motor*> left = {
    &frontLeft, &backLeft
};
std::vector<lemlib::Motor*> right = {
    &frontRight, &backRight
};
```

### 2-Motor Tank Drive

```cpp
// Single motor per side
std::vector<lemlib::Motor*> left = { &leftMotor };
std::vector<lemlib::Motor*> right = { &rightMotor };
```

### 6-Motor Drive

```cpp
// 3 motors per side
std::vector<lemlib::Motor*> left = {
    &frontLeft, &midLeft, &backLeft
};
std::vector<lemlib::Motor*> right = {
    &frontRight, &midRight, &backRight
};
```

## Troubleshooting

### Problem: Motor Spins Wrong Direction

**Solution:** Enable `reversed` flag
```cpp
lemlib::V5Motor motor(port, true);  // Reverse this motor
```

### Problem: Tracking Wheel Not Responding

**Solution:** Check ADI ports
```cpp
// Verify port letters (A, B, etc)
lemlib::ADIEncoder enc('A', 'B');  // Correct

// And pinout on expander
```

### Problem: IMU Won't Calibrate

**Solution:** 
- Don't rotate robot during calibration
- Ensure sensor port is correct
- Check physical connection

### Problem: Motors Not Responding

**Solution:**
- Check port numbers (1-21)
- Verify motor reverse flag if needed
- Check motor cables

## Best Practices

1. **Standardize port layout** across team
2. **Document reversal flags** in code
3. **Test motor directions** in operator control first
4. **Calibrate sensors** before autonomous
5. **Use MotorGroups** for consistent left/right behavior
6. **Monitor feedback** for encoder health

## See Also

- [Configuration](../tutorials/2_configuration.md) - Setting up motor groups
- [Odometry](./odometry.md) - Using feedback sensors
- [Motion Algorithms](./motions.md) - Using motor groups
- [PROS Motor API](https://pros.cs.purdue.edu/v5/api/cpp/motors.html)
