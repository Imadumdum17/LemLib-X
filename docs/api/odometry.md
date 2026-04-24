# Odometry and Tracking Documentation

## Overview

Odometry is the process of tracking the robot's position and orientation by integrating motion data from sensors. LemLib-X's `TrackingWheelOdom` class provides precise position tracking using tracking wheels and IMU fusion.

## Why Odometry Matters

Without odometry, the robot cannot:
- Know where it is on the field
- Calculate vectors to targets
- Execute Pure Pursuit path following
- Maintain positioning autonomy

Odometry provides the "source of truth" for motion algorithms.

## Odometry Sensors

### Primary: Tracking Wheels

Dedicated, non-driven wheels measure lateral and forward movement.

**Advantages:**
- High accuracy (±0.5 inches possible)
- Immune to drivetrain slip
- Parallel arrangement prevents center-of-rotation errors

**Configuration:**
```cpp
// Horizontal tracking wheel (perpendicular to drive)
// Measures left-right movement
lemlib::Encoder horizontalEncoder(/* params */);

// Vertical tracking wheel (parallel to drive)  
// Measures forward-backward movement
lemlib::Encoder verticalEncoder(/* params */);
```

### Secondary: IMU (Inertial Measurement Unit)

Measures heading (rotation angle).

**Advantages:**
- Direct heading measurement
- Eliminates rotation errors from tracking wheels
- Stable over long periods

**Configuration:**
```cpp
lemlib::IMU gyro(/* serial port */);
```

### Fallback: Motor Encoders

Built-in motor encoders provide basic odometry when tracking wheels unavailable.

**Limitations:**
- Affected by wheel slip
- Accumulates error over time
- Prone to calibration drift

## TrackingWheelOdom Class

### Overview

```cpp
class TrackingWheelOdom {
    public:
        // Update robot position based on sensor data
        void update();
        
        // Get current position and heading
        units::Pose getPose();
        
        // Reset to known position
        void setPose(units::Pose newPose);
};
```

### Constructor

```cpp
TrackingWheelOdom(
    Encoder& horizontalTracking,    // Left-right movement
    Encoder& verticalTracking,      // Forward-backward movement
    IMU& inertialSensor,            // Heading measurement
    Length trackWidth               // Distance between wheel axles
)
```

**Parameters:**
- `horizontalTracking`: Encoder measuring perpendicular (lateral) motion
- `verticalTracking`: Encoder measuring parallel (forward) motion
- `inertialSensor`: IMU for heading correction
- `trackWidth`: Distance between drive motor wheels (affects rotation accuracy)

### Setup Example

```cpp
// In main.cpp initialization:

// Create tracking wheel encoders
lemlib::ADIEncoder horizontalEncoder('A', 'B');  // Ports A, B
lemlib::ADIEncoder verticalEncoder('C', 'D');    // Ports C, D

// Create IMU
lemlib::V5InertialSensor imu(10);               // Port 10

// Create odometry tracker
static lemlib::TrackingWheelOdom odom(
    horizontalEncoder,
    verticalEncoder,
    imu,
    8_in                                         // 8 inch track width
);

// Provide as pose getter for motions
lemlib::units::Pose getPose() {
    return odom.getPose();
}
```

### Methods

#### `void update()`

Updates position estimate based on latest sensor data.

**Call this regularly** - typically in a background task or motion loop.

```cpp
// In background task
void trackingTask() {
    while (true) {
        odom.update();                    // Update position
        
        auto pose = odom.getPose();
        printf("X=%.2f Y=%.2f H=%.2f\n",
               pose.x.convert(inch),
               pose.y.convert(inch),
               pose.theta.convert(degree));
        
        pros::delay(10);                  // Update at 100 Hz
    }
}
```

#### `Pose getPose()`

Returns current position and heading.

**Returns:** `Pose` struct with x, y coordinates and theta heading

```cpp
auto pose = odom.getPose();
std::cout << "Position: (" << pose.x << ", " << pose.y << ")" << std::endl;
std::cout << "Heading: " << pose.theta << std::endl;
```

#### `void setPose(Pose newPose)`

Resets odometry to known position (e.g., at match start).

```cpp
void autonomous() {
    // Reset to known starting position
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    // Robot now thinks it's at origin facing forward
}
```

## Pose Struct

```cpp
struct Pose {
    Length x;            // X position (inches)
    Length y;            // Y position (inches)
    Angle theta;         // Heading (degrees)
    
    // Constructors
    Pose(Length x, Length y, Angle theta);
    Pose();              // Default: (0, 0, 0)
};
```

### Usage

```cpp
// Create pose
lemlib::Pose targetPose(24_in, 36_in, 45_deg);

// Access components
std::cout << "X: " << targetPose.x.convert(inch) << std::endl;
std::cout << "Y: " << targetPose.y.convert(inch) << std::endl;
std::cout << "Heading: " << targetPose.theta.convert(degree) << std::endl;
```

## Coordinate System

LemLib-X uses a standard field coordinate system:

```
                90°
               (N)
                ▲
    (W) 180°    │    0° (E)
                │
    ────────────┼────────────
                │
                │
                ▼
              270° (S)

X-axis increases to the right (East)
Y-axis increases upward (North)
Theta increases counter-clockwise
```

### Field Positions

```cpp
// Bottom-left corner
lemlib::Pose(0, 0, 0_deg);

// Center of field
lemlib::Pose(72, 72, 0_deg);  // 144x144 field

// Top-right corner
lemlib::Pose(144, 144, 90_deg);
```

## Drift Compensation

Over time, odometry errors accumulate:
- Tracking wheels: mechanical wear, surface friction
- IMU: temperature drift, gyro bias

### Automatic Compensation

LemLib continuously:
1. Fuses tracking wheel data with IMU
2. Detects inconsistencies
3. Applies drift correction

```cpp
extern const Number drift_compensation;  // From config.hpp

// Adjust to balance tracking wheel vs IMU trust
// Higher = trust IMU more
// Lower = trust tracking wheels more
```

### Manual Correction

Periodically re-calibrate:

```cpp
void autonomous() {
    odom.setPose(odom.getPose());  // Clear accumulated drift
    
    // Or reset to known position
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
}
```

## Sensor Configuration

### Tracking Wheel Optimal Setup

```
       Front
         ▲
         │
    ●────┼────●  ← Horizontal wheel
         │
    ┌────┼────┐
    │ Drive  │
    │ Motors │
    │────────│
         │
    ●────┼────●  ← Vertical wheel (perpendicular)
         │
```

**Key points:**
- Horizontal wheel perpendicular to drive direction
- Vertical wheel parallel to drive direction
- Both wheels centered on robot
- Mounted on low-friction, accurate bearings

### Encoder Setup

```cpp
// For each tracking wheel encoder:

// ADI (3-wire) Encoder
lemlib::ADIEncoder trackingEnc(portA, portB);

// V5 Rotation Sensor (built-in)
lemlib::V5RotationSensor sensorEnc(port11);
```

### IMU Setup

```cpp
// V5 Inertial Sensor
lemlib::V5InertialSensor imu(port10);

// Calibrate at startup
imu.calibrate();
// Wait for calibration to complete
while (imu.isCalibrating()) {
    pros::delay(10);
}
```

## Calibration

### Tracking Wheel Calibration

```cpp
// Test procedure:
// 1. Start at origin (0, 0)
// 2. Drive exactly 12 inches forward
// 3. Check reported distance

// If odom reports:
// - 12.5 in: Encoders are slightly off, needs calibration
// - 11.5 in: Encoders need adjustment

// Adjust encoder gear ratios or scaling factors
```

### IMU Calibration

```cpp
// In initialization:
imu.calibrate();

// Wait for completion (2-3 seconds)
while (imu.isCalibrating()) {
    pros::delay(10);
}

// Now ready to use - do NOT rotate robot during calibration
```

## Common Integration Points

### As Pose Getter for Motions

```cpp
// In config.hpp
extern const std::function<units::Pose()> pose_getter;

// In main.cpp
const std::function<units::Pose()> pose_getter = []() {
    return odom.getPose();
};

// Motion algorithms automatically call this
```

### In Custom Motion Loops

```cpp
void customMotion() {
    std::uint32_t prevTime = pros::millis();
    
    while (!pros::Task::notify_take(true, 0)) {
        // Update odometry
        odom.update();
        
        // Get current position
        auto pose = odom.getPose();
        
        // Calculate error to target
        lemlib::Pose target(48, 48, 45_deg);
        Number distance = 
            std::sqrt(pow(pose.x - target.x) + 
                     pow(pose.y - target.y));
        
        // Use in motion control
        Number output = pid.update(distance);
        motors.moveVoltage(output);
        
        pros::Task::delay_until(&prevTime, 10);
    }
}
```

### Debugging and Monitoring

```cpp
// Print odometry data
void printOdom() {
    auto pose = odom.getPose();
    printf("Pose: (%.2f, %.2f, %.2f°)\n",
           pose.x.convert(inch),
           pose.y.convert(inch),
           pose.theta.convert(degree));
}

// In autonomous
void autonomous() {
    odom.setPose(lemlib::Pose(0, 0, 0_deg));
    
    printOdom();  // Should print: (0.00, 0.00, 0.00°)
    
    moveForward(12_in);
    
    printOdom();  // Should print roughly: (12.00, 0.00, 0.00°)
}
```

## Troubleshooting

### Problem: Odometry Drift (Position gradually wrong)

**Cause:** Tracking wheels not calibrated or IMU biased  
**Solution:**
- Calibrate tracking wheel distances
- Recalibrate IMU
- Check drift_compensation parameter

### Problem: Inconsistent Position Reports

**Cause:** Encoder noise or loose mechanical connections  
**Solution:**
- Secure tracking wheels firmly
- Check encoder cables
- Add noise filtering to encoder readings

### Problem: Heading Always Wrong

**Cause:** IMU misconfigured or not calibrated  
**Solution:**
- Re-run `imu.calibrate()`
- Verify IMU serial port
- Ensure robot is still during calibration

### Problem: Position Jumps Suddenly

**Cause:** Drift compensation algorithm overcorrecting  
**Solution:**
- Adjust drift_compensation parameter
- Check if tracking wheels are slipping
- Verify IMU quality

## Best Practices

1. **Initialize at startup:**
   ```cpp
   imu.calibrate();
   odom.setPose(lemlib::Pose(0, 0, 0_deg));
   ```

2. **Update regularly:**
   ```cpp
   // Every 10ms in motion loop or background task
   odom.update();
   ```

3. **Reset between matches:**
   ```cpp
   odom.setPose(lemlib::Pose(0, 0, 0_deg));  // Known starting position
   ```

4. **Verify calibration:**
   - Drive known distances
   - Check reported positions
   - Test heading accuracy

5. **Monitor drift:**
   - Log odometry data
   - Detect patterns of error
   - Recalibrate sensors if needed

6. **Use proper coordinate system:**
   - Keep origin at known point
   - Use consistent heading references
   - Document field coordinate system

## See Also

- [Hardware Integration](./hardware.md) - Encoder and IMU details
- [Motion Algorithms](./motions.md) - Use odometry for positioning
- [Configuration](../tutorials/2_configuration.md) - Setting up pose_getter
- [Tutorials](../tutorials/index.md) - Practical examples
