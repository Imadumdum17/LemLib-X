# Utilities and Helper Functions

## Overview

LemLib-X provides utility functions and classes for common robotics tasks like unit conversions, vector math, timer management, and motion cancellation.

## Timer Utilities

### Timer Class

Manages elapsed time for motion sequences.

```cpp
class Timer {
    public:
        Timer();
        
        // Start the timer
        void start();
        
        // Check elapsed time
        Time getElapsedTime() const;
        
        // Check if timeout reached
        bool isTimeout(Time timeout) const;
        
        // Reset timer
        void reset();
        
        // Pause/Resume
        void pause();
        void resume();
};
```

**Usage:**

```cpp
void timeoutMotion() {
    lemlib::Timer timeout;
    timeout.start();
    
    while (timeout.getElapsedTime() < 5000_msec) {
        // Motion logic
        
        // Or check with isTimeout:
        if (timeout.isTimeout(5000_msec)) {
            break;  // Time's up
        }
    }
}
```

## Vector Mathematics

### Vector2D

2D vector for position, velocity, or field coordinates.

```cpp
class Vector2D {
    public:
        Length x;
        Length y;
        
        Vector2D();
        Vector2D(Length x, Length y);
        
        // Distance from origin
        Length magnitude() const;
        
        // Angle to this vector
        Angle angle() const;
        
        // Vector operations
        Vector2D operator+(const Vector2D& other) const;
        Vector2D operator-(const Vector2D& other) const;
        Vector2D operator*(Number scalar) const;
};
```

**Usage:**

```cpp
// Robot position
lemlib::Vector2D pos(24_in, 36_in);

// Target
lemlib::Vector2D target(48_in, 60_in);

// Calculate vector to target
lemlib::Vector2D error = target - pos;

// Distance to target
Length distance = error.magnitude();

// Angle to target
Angle heading = error.angle();

printf("Distance: %.2f inches\n", distance.convert(inch));
printf("Direction: %.2f degrees\n", heading.convert(degree));
```

### Vector3D

3D vector for advanced applications.

```cpp
class Vector3D {
    public:
        Length x, y, z;
        
        Vector3D(Length x, Length y, Length z);
        
        Length magnitude() const;
        Vector3D operator+(const Vector3D& other) const;
        Vector3D operator-(const Vector3D& other) const;
};
```

## Unit System

### Angle

Angle representation with automatic conversions.

```cpp
// Create angles in degrees
Angle a = 90_deg;

// Or radians
Angle b = 1.57_rad;

// Convert to other units
float degrees = a.convert(degree);  // 90.0
float radians = a.convert(radian);  // ~1.57
float revolutions = a.convert(revolution);  // 0.25

// Arithmetic
Angle c = a + b;
Angle d = a - b;

// Comparison
if (a > 45_deg) { /* ... */ }
if (b == 90_deg) { /* ... */ }
```

### Length

Distance representation.

```cpp
// Create lengths in inches
Length a = 24_in;

// Or centimeters
Length b = 60_cm;

// Or feet
Length c = 2_ft;

// Convert
float inches = a.convert(inch);  // 24.0
float cm = a.convert(cm);        // ~60.96
float meters = a.convert(meter); // ~0.6096

// Arithmetic
Length d = a + b;
Length e = a * 2;

// Special values
if (a > 12_in && a < 48_in) { /* ... */ }
```

### Time

Time intervals.

```cpp
// Create times in milliseconds
Time t1 = 200_msec;

// Or seconds
Time t2 = 5_sec;

// Or minutes
Time t3 = 1_min;

// Convert
int ms = t1.convert(msec);   // 200
float sec = t2.convert(second);  // 5.0

// Arithmetic
Time t4 = t1 + t2;
Time t5 = t2 * 2;  // 10 seconds

// Wait
pros::delay(t1.convert(msec));
```

## Pose and Position Utilities

### Pose

Complete position and heading.

```cpp
struct Pose {
    Length x;
    Length y;
    Angle theta;
    
    Pose();                                    // (0, 0, 0°)
    Pose(Length x, Length y);                 // (x, y, 0°)
    Pose(Length x, Length y, Angle theta);    // Full pose
    
    // Distance to another pose
    Length distance(const Pose& other) const;
    
    // Heading toward another pose
    Angle heading(const Pose& other) const;
};
```

**Usage:**

```cpp
// Current position
lemlib::Pose current(12_in, 24_in, 45_deg);

// Target
lemlib::Pose target(48_in, 72_in, 90_deg);

// Calculate distance to target
Length dist = current.distance(target);

// Calculate heading to target
Angle heading = current.heading(target);

// Calculate error
Angle headingError = target.theta - current.theta;
```

## Normalized Angle Helper

Normalize angles to -180° to +180° range.

```cpp
// Utilities for angle calculations
Angle normalize(Angle angle);  // Returns -180 to +180

// Examples
Angle a = 270_deg;
Angle normalized = normalize(a);  // -90_deg

Angle b = -190_deg;
normalized = normalize(b);  // 170_deg
```

## Motion Cancellation Helper

### MotionCancelHelper

Simplifies checking motion cancellation in custom motions.

```cpp
bool isCancellationRequested();
```

**Usage:**

```cpp
void customMotion() {
    while (!isCancellationRequested()) {
        // Motion logic
    }
    
    // Cleanup on exit
    motors.moveVoltage(0);
}

// Instead of:
void customMotionVerbose() {
    while (!pros::Task::notify_take(true, 0)) {
        // motion logic
        std::cout << "Moving..." << std::endl;
    }
}
```

## Math Utilities

### abs (Absolute Value)

Get magnitude of quantity.

```cpp
Length dist = -24_in;
Length magnitude = abs(dist);  // 24_in

Angle angle = -45_deg;
Angle absAngle = abs(angle);   // 45_deg
```

### sqrt, pow, etc.

Standard math functions work with quantities.

```cpp
// Distance calculation
Length dx = 10_in;
Length dy = 20_in;
Length hypotenuse = sqrt(dx*dx + dy*dy);

// Velocity calculation
Length distance = 24_in;
Time time = 2_sec;
// Speed = distance/time
```

## Debugging and Logging

### Print Helpers

Print quantities in readable format.

```cpp
Pose pos(24_in, 36_in, 45_deg);

// Direct printing
std::cout << "Position: " << pos.x << ", " << pos.y 
          << " | Heading: " << pos.theta << std::endl;

// Or use format functions
printf("X: %.2f, Y: %.2f, Theta: %.2f\n",
       pos.x.convert(inch),
       pos.y.convert(inch),
       pos.theta.convert(degree));
```

### Assert Macros

Verify conditions during testing.

```cpp
// Verify robot initialized properly
assert(motor != nullptr);
assert(odom.getPose().x == 0_in);
```

## Common Utility Patterns

### Distance Calculation

```cpp
// Between two poses
lemlib::Pose p1(0_in, 0_in);
lemlib::Pose p2(24_in, 36_in);
Length distance = p1.distance(p2);  // ~43.27 inches
```

### Heading Calculation

```cpp
// Direction from robot to target
lemlib::Pose robot(10_in, 10_in, 0_deg);
lemlib::Pose target(20_in, 20_in);

Angle heading = robot.heading(target);  // ~45_deg
```

### Timeout Detection

```cpp
const uint32_t START_TIME = pros::millis();
const uint32_t TIMEOUT = 5000;  // ms

while ((pros::millis() - START_TIME) < TIMEOUT) {
    // Motion logic
}
```

### Rate Limiting

```cpp
uint32_t prevTime = pros::millis();
const uint32_t LOOP_TIME = 10;  // ms

while (true) {
    // Consistent update rate
    pros::Task::delay_until(&prevTime, LOOP_TIME);
    
    // Execute at ~100 Hz
    updateMotors();
}
```

## Error Handling

### Null Checks

```cpp
// Always verify pointers
if (motor == nullptr) {
    std::cerr << "Motor not initialized!" << std::endl;
    return;
}

// Or use assertions in debug
assert(motor != nullptr);
```

### Boundary Validation

```cpp
// Verify ports are valid
if (port < 1 || port > 21) {
    std::cerr << "Invalid port: " << port << std::endl;
    return false;
}
```

## Performance Considerations

### Memory Efficiency

```cpp
// Stack-allocated vectors (efficient)
lemlib::Vector2D pos(24_in, 36_in);

// Avoid unnecessary copies
const lemlib::Pose& getPoseRef() { return currentPose; }
auto& pose = getPoseRef();  // Reference, no copy
```

### Computation Efficiency

```cpp
// Expensive operations (avoid in loops)
Length dist = pos.distance(target);  // sqrt() call

// Cheaper operations (prefer in loops)
Length dx = pos.x - target.x;
Length dy = pos.y - target.y;
```

## Type Safety Benefits

```cpp
// Type system prevents mistakes
Length inches = 24_in;
Angle degrees = 90_deg;

// This won't compile (good!)
// Length result = inches + degrees;  // ERROR!

// Correct mixing of compatible types
Length total = 24_in + 12_in;  // OK
```

## See Also

- [Units System](./api/units.md) - Complete unit documentation
- [Math Reference](./api/math.md) - Detailed math function reference
- [PID Controller](./api/pid.md) - Timer usage in control loops
- [Motion Algorithms](./api/motions.md) - Pose usage in motions
