# PID Controller Documentation

## Overview

The PID (Proportional-Integral-Derivative) controller is the core control algorithm used by LemLib-X for motion management. It processes error signals and generates proportional correction outputs to move the robot toward target states.

## What is PID Control?

PID is a feedback control loop that calculates a correction based on three components:

- **Proportional (P)**: Responds to current error - higher gains create faster responses but can cause overshoot
- **Integral (I)**: Accumulates historical error - helps eliminate steady-state error but can cause instability if too high
- **Derivative (D)**: Responds to rate of error change - dampens oscillations and reduces overshoot

### Control Equation

```
output = (kP × error) + (kI × integral_error) + (kD × error_rate)
```

Where:
- `kP`, `kI`, `kD` are tuning gains
- `error` is the difference between target and current value
- `integral_error` is the accumulated error over time
- `error_rate` is how fast the error is changing

## Class Reference

### Constructors

#### BasicConstructor

```cpp
PID(Number kP, Number kI, Number kD, 
    Number windupRange = 0, bool signFlipReset = false)
```

Creates a PID controller with individual gain coefficients.

**Parameters:**
- `kP`: Proportional gain
- `kI`: Integral gain
- `kD`: Derivative gain
- `windupRange`: Maximum integral accumulation before resetting (default: 0 = disabled)
- `signFlipReset`: Reset integral when error crosses zero (default: false)

**Example:**
```cpp
// Create PID with moderate tuning
lemlib::PID pid(1.0, 0.1, 0.01, 10, true);
```

#### Gains Struct Constructor

```cpp
PID(const Gains& gains, 
    Number windupRange = 0, bool signFlipReset = false)
```

Creates a PID controller using a `Gains` structure for cleaner code.

**Parameters:**
- `gains`: `Gains` struct containing kP, kI, kD values
- `windupRange`: Maximum integral accumulation (default: 0)
- `signFlipReset`: Reset integral when error changes sign (default: false)

**Example:**
```cpp
lemlib::Gains gains = {1.0, 0.1, 0.01};
lemlib::PID pid(gains, 10, true);
```

### Methods

#### `Gains getGains()`

Retrieves the current PID gains.

**Returns:** `Gains` struct with current kP, kI, kD values

**Example:**
```cpp
lemlib::Gains currentGains = pid.getGains();
std::cout << "kP: " << currentGains.kP << std::endl;
```

#### `void setGains(Gains gains)`

Updates the PID gains at runtime.

**Parameters:**
- `gains`: New `Gains` struct to apply

**Example:**
```cpp
lemlib::Gains highGains = {2.0, 0.2, 0.05};
pid.setGains(highGains);
```

#### `Number update(Number error)`

Updates the PID controller with new error and returns the control output.

**Call this repeatedly in your control loop.** This function:
- Calculates the derivative from the previous error
- Accumulates integral with windup protection
- Outputs the combined P+I+D correction

**Parameters:**
- `error`: Difference between setpoint and current value (`error = setpoint - current`)

**Returns:** Control signal (typically -127 to 127 for motor voltage, or -600 to 600 for motor velocity)

**Example:**
```cpp
// In a 10 ms control loop
while (robot.isMoving()) {
    Number currentPos = robot.getPosition();
    Number targetPos = 500;
    Number error = targetPos - currentPos;
    
    Number output = pid.update(error);
    
    motor.moveVoltage(output);
    pros::delay(10);
}
```

#### `void reset()`

Clears all internal state (integral accumulation and previous error).

**Use before starting a new motion** to prevent integration windup or carrying over errors from previous movements.

**Example:**
```cpp
void moveToNewTarget(Number target) {
    pid.reset();  // Clear previous state
    
    while (!settled) {
        Number error = target - getCurrentPos();
        motor.moveVoltage(pid.update(error));
    }
}
```

## Gains Structure

```cpp
struct Gains {
    Number kP = 0;  // Proportional gain
    Number kI = 0;  // Integral gain
    Number kD = 0;  // Derivative gain
};
```

### Usage

```cpp
// Create gains
lemlib::Gains myGains;
myGains.kP = 1.5;
myGains.kI = 0.2;
myGains.kD = 0.05;

// Or initialize directly
lemlib::Gains quickGains = {1.5, 0.2, 0.05};

// Create PID with these gains
lemlib::PID controller(myGains);
```

## Tuning Guide

### Tuning Process

1. **Start Conservative**: Begin with `kP = 0.5`, `kI = 0`, `kD = 0`

2. **Increase Proportional Gain (kP)**:
   - Gradually increase until system responds quickly but oscillates
   - Then back off slightly
   - If system is too sluggish, increase kP more
   - If system overshoots significantly, decrease kP

3. **Add Derivative Gain (kD)**:
   - Helps dampen oscillations
   - Typical range: 0.05 to 0.2 × kP
   - Reduces overshoot without slowing response
   - Too high kD amplifies sensor noise

4. **Add Integral Gain (kI)**:
   - Use small values (typically 0.01 to 0.1 × kP)
   - Helps eliminate final steady-state error
   - Too high kI causes instability and oscillation
   - Consider enabling `signFlipReset` to prevent integral buildup during direction changes

### Tuning Strategy Example

```cpp
// Initial conservative tuning
lemlib::PID lateralPID(0.5, 0, 0);        // Pure proportional

// Later, after observing response, refine:
lateralPID.setGains({1.2, 0.05, 0.1});   // Add I and D

// Use edit pages to dynamically adjust without recompiling
```

### Parameter Meanings

| Parameter | Effect | Too High | Too Low |
|-----------|--------|----------|---------|
| **kP** | Responsiveness | Oscillation & overshoot | Sluggish response, misses target |
| **kI** | Steady-state error removal | Instability & overshoot | Slow final approach to target |
| **kD** | Oscillation damping | Jerky motion, noise sensitive | Underdamped, overshoot |

### Special Parameters

#### `windupRange`

Prevents integral error from growing too large when the system can't reach the target.

```cpp
// Without windup protection, integral accumulation causes overshoot
lemlib::PID pid(1.0, 0.1, 0.01, 0);       // Unlimited accumulation

// With windup protection, integral resets when output reaches limit
lemlib::PID pid(1.0, 0.1, 0.01, 50);      // Reset if accumulated > 50
```

**Use when:**
- System has physical limits (motor max voltage)
- Large steady-state errors that can't be corrected

#### `signFlipReset`

Automatically resets integral accumulation when error changes sign.

```cpp
// Useful for oscillating systems
lemlib::PID pid(1.0, 0.1, 0.01, 10, true);

// Without this: integral keeps building when error crosses zero
// With this: integral clears when switching from positive to negative error
```

**Use when:**
- Controller overshoots and oscillates around setpoint
- Error frequently changes sign

## Common Problems and Solutions

### Problem: Robot Oscillates Constantly
**Cause:** kP is too high or kD is too low  
**Solution:** Reduce kP by 20%, increase kD by 50%

### Problem: Robot Never Reaches Target
**Cause:** kP is too low, integral is needed  
**Solution:** Increase kP, add small kI (0.01 × kP)

### Problem: Robot Overshoots Target Significantly
**Cause:** kD is too low or kP is too high  
**Solution:** Increase kD (2-3× current value), reduce kP slightly

### Problem: Robot Jerks/Jumps in Motion
**Cause:** kD too high or noisy sensor readings  
**Solution:** Reduce kD, ensure clean encoder/IMU readings

### Problem: Takes Forever to Correct Large Errors
**Cause:** kP is too low  
**Solution:** Increase kP systematically in 0.1 increments

## Real-World Usage in LemLib-X

### Lateral Control

```cpp
// In config.hpp - controls forward/backward movement
extern lemlib::PID lateral_pid;

// Used by motion algorithms:
Number lateralError = target_distance - current_distance;
Number lateralOutput = lateral_pid.update(lateralError);
leftMotors.moveVoltage(lateralOutput);
rightMotors.moveVoltage(lateralOutput);
```

### Angular Control

```cpp
// In config.hpp - controls robot rotation
extern lemlib::PID angular_pid;

// Used by motion algorithms:
Number angularError = target_angle - current_angle;
Number angularOutput = angular_pid.update(angularError);
leftMotors.moveVoltage(-angularOutput);
rightMotors.moveVoltage(angularOutput);
```

## Best Practices

1. **Call `reset()` between motions** to clear accumulated state
2. **Use symmetric gains** for balanced left/right motion
3. **Start small with kI** - integral gain causes most tune problems
4. **Profile your system** before tuning - understand response curves
5. **Log error values** during development for debugging
6. **Use the PageManager** to tune gains in real-time during testing
7. **Test on different parts of field** - friction/battery level affect response
8. **Document your tuned values** for team reference

## Mathematical Reference

### Continuous PID Equation

```
U(t) = kP × e(t) + kI × ∫e(t)dt + kD × de(t)/dt
```

### Discrete Implementation (as used in LemLib-X)

```
U[n] = kP × e[n] + kI × Σe[n] + kD × (e[n] - e[n-1])/dt
```

Where:
- `n` = current time step
- `dt` = time since last update (typically 10 ms)

## See Also

- [Motion Algorithms](./motions.md) - How PID is used in moveToPoint, follow, etc.
- [ExitCondition](./exit-condition.md) - Works with PID to detect when motion is complete
- [Configuration](../tutorials/2_configuration.md) - Setting up PID in config.hpp
