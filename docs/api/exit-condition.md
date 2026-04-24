# Exit Condition Documentation

## Overview

Exit Conditions define when a motion should stop. They use a timer-based approach: the motion ends when the error stays within a target range for a minimum duration, or when a timeout is reached.

## Core Concept

An exit condition is **satisfied** when:
1. The error is within a defined `range` **AND**
2. The error has been within that range for at least `time` duration

This prevents premature motion termination due to brief sensor fluctuations.

## Class Reference

### ExitCondition Template Class

```cpp
template <isQuantity Q> class ExitCondition
```

The template parameter `Q` specifies the quantity type (e.g., `Length`, `Angle`, `Voltage`).

#### Constructor

```cpp
ExitCondition(Q range, Time time)
```

**Parameters:**
- `range`: Maximum error magnitude before timer resets. Error must be within [-range, +range]
- `time`: How long the error must stay within range before condition is satisfied

**Example:**
```cpp
// Motion ends when error is within 5 inches for 200 milliseconds
lemlib::ExitCondition<Length> lateralExit(5_in, 200_msec);

// Motion ends when angular error is within 2 degrees for 150 milliseconds
lemlib::ExitCondition<Angle> angularExit(2_deg, 150_msec);
```

#### `bool update(Q input)`

Tests if the exit condition is met based on current error.

**Parameters:**
- `input`: The current error value to check

**Returns:** `true` if condition is satisfied, `false` otherwise

**Behavior:**
- If `|input| >= range`: Timer resets (action starts over)
- If `|input| < range`: Timer counts up
- When elapsed time reaches `time`: Returns `true` and stays `true`

**Example:**
```cpp
// In motion loop (runs every 10ms)
while (true) {
    Number error = target - current;
    
    if (exit.update(error)) {
        break;  // Motion complete!
    }
    
    pros::delay(10);
}
```

#### `void reset()`

Clears all internal state (timer and satisfied flag).

**Use:** Call before starting a new motion to ensure the timer starts fresh.

**Example:**
```cpp
void moveToPoint(Point target) {
    exit.reset();  // Clear previous state
    
    // Motion loop...
}
```

## ExitConditionGroup

When a motion has multiple exit criteria, use `ExitConditionGroup`:

```cpp
template <isQuantity Q>
class ExitConditionGroup {
    // Typically contains:
    // - Timeout condition
    // - Lateral (position) settling condition
    // - Angular (heading) settling condition
};
```

A motion ends when **any** condition in the group is satisfied.

### Usage in Config

```cpp
// In config.hpp
extern const lemlib::ExitConditionGroup<Length> lateral_exit_conditions;
extern const lemlib::ExitConditionGroup<Angle> angular_exit_conditions;

// Defined in implementation file:
const lemlib::ExitConditionGroup<Length> lateral_exit_conditions = {
    lemlib::ExitCondition<Length>(2_in, 200_msec),    // Settled condition
    lemlib::ExitCondition<Length>(5_in, 1000_msec),   // Looser backup
    lemlib::ExitCondition<Length>(100_in, 5000_msec)  // Timeout
};
```

### Checking Multiple Conditions

```cpp
// Motion ends when lateral OR angular condition is met
if (lateral_exit_conditions.update(lateralError) || 
    angular_exit_conditions.update(angularError)) {
    motion_complete = true;
}
```

## Timeout Handling

Most motions have a timeout as a safety mechanism:

```cpp
// Timeout is a separate exit condition that's always satisfied after 5 seconds
lemlib::ExitCondition<Time> timeout(100_msec, 5000_msec);

// Equivalently:
TimeT timeoutStart = pros::millis();
while ((pros::millis() - timeoutStart) < 5000) {
    // Motion runs...
}
```

## Motion-Specific Exit Conditions

### moveToPoint - Lateral Motion Only

```cpp
// Motion ends when:
// - Robot is within 2 inches of target for 200ms, OR
// - 5 seconds elapse
lemlib::ExitCondition lateralExit(2_in, 200_msec);

// In motion:
while (true) {
    Number error = target_distance - current_distance;
    
    if (lateralExit.update(error) || timeoutExpired) {
        break;
    }
    
    pros::delay(10);
}
```

### moveToPose - Lateral and Angular

```cpp
// Both must converge:
lemlib::ExitCondition<Length> lateralExit(2_in, 200_msec);
lemlib::ExitCondition<Angle> angularExit(2_deg, 200_msec);

while (true) {
    Number lateralError = distanceToTarget();
    Number angularError = angleToTarget();
    
    // Move ends when BOTH are settled OR timeout
    if ((lateralExit.update(lateralError) && 
         angularExit.update(angularError)) || 
        timeoutExpired) {
        break;
    }
    
    pros::delay(10);
}
```

### follow - Path Following

```cpp
// Exits when heading matches path end angle
lemlib::ExitCondition<Angle> angularExit(1_deg, 100_msec);

while (true) {
    Number headingError = calculateHeadingError();
    
    if (angularExit.update(headingError) || 
        pathComplete || timeoutExpired) {
        break;
    }
    
    pros::delay(10);
}
```

## Tuning Exit Conditions

### Range Parameter

Defines how close the robot must get to the target.

```cpp
// Tight tolerance - stops only when very close
lemlib::ExitCondition tight(0.5_in, 200_msec);

// Loose tolerance - stops when reasonably close
lemlib::ExitCondition loose(3_in, 200_msec);
```

**Considerations:**
- Smaller range → more accurate but takes longer
- Larger range → faster but less accurate
- Must be achievable with your PID tuning
- Too tight → motion may never satisfy (oscillates around target)

### Time Parameter

How long the error must stay within range before finishing.

```cpp
// Quick exit - short settle time
lemlib::ExitCondition quick(2_in, 50_msec);

// Stable exit - long settle time
lemlib::ExitCondition stable(2_in, 300_msec);
```

**Considerations:**
- Shorter time → motion ends faster but may be incomplete
- Longer time → ensures stability but uses more autonomous time
- Must balance accuracy vs. speed goal
- Sensor noise can cause brief excursions - don't set too short

### Typical Configurations

| Motion Type | Range | Time | Purpose |
|---|---|---|---|
| **Precise positioning** | 1-2 units | 200-300 ms | High accuracy, reasonable speed |
| **Fast movement** | 3-5 units | 100-150 ms | Speed over precision |
| **Stabilization** | 0.5-1 unit | 300-500 ms | Maximum accuracy |
| **Recovery** | 5-10 units | 150-200 ms | Fallback precision |

## Common Issues and Solutions

### Problem: Motion Stops Too Early
**Cause:** Range too large or time too short  
**Solution:** 
- Increase `range` to require settling closer to target
- Increase `time` to require longer stability
- Check if sensor noise is causing false triggers

### Problem: Motion Never Stops (Until Timeout)
**Cause:** Range too small (can't achieve target) or time too long  
**Solution:**
- Tune PID first to ensure robot can reach targets
- Increase `range` to be realistic for achievable accuracy
- Reduce `time` if you're confident in settling

### Problem: Motion Completes But Robot Continues Drifting
**Cause:** PID can't hold position, or integral windup  
**Solution:**
- Increase lateral PID kP for better holding
- Reduce kI to prevent accumulated error
- Add kD for damping

### Problem: Oscillation Detection (Robot Oscillates After Motion)
**Cause:** Exit condition time too short, motion "whips" past target  
**Solution:**
- Increase settle time (`time` parameter)
- Reduce lateral PID kP to round out approach
- Increase kD for damping

## Advanced Usage

### Dynamic Exit Conditions

Change conditions mid-motion:

```cpp
void adaptiveMove() {
    lemlib::ExitCondition<Length> exit(5_in, 200_msec);
    
    // Start moving...
    while (!motion_complete) {
        if (isCloseToTarget()) {
            // Switch to tighter tolerance
            exit = lemlib::ExitCondition<Length>(1_in, 300_msec);
        }
        
        if (exit.update(error)) {
            break;
        }
    }
}
```

### Multiple Timeout Conditions

Stack conditions for graceful degradation:

```cpp
lemlib::ExitConditionGroup<Length> multiExit = {
    lemlib::ExitCondition<Length>(1_in, 200_msec),    // Ideal
    lemlib::ExitCondition<Length>(3_in, 500_msec),    // Acceptable
    lemlib::ExitCondition<Length>(100_in, 3000_msec)  // Timeout
};

// Settles perfectly if possible, but doesn't wait forever
```

### Sensor-Aware Conditions

Different conditions based on confidence:

```cpp
if (hasTrackingWheels) {
    exit = lemlib::ExitCondition<Length>(1_in, 200_msec);  // Accurate sensor
} else {
    exit = lemlib::ExitCondition<Length>(3_in, 300_msec);  // Less reliable
}
```

## Best Practices

1. **Set range achievable**: Range should be achievable with well-tuned PID
2. **Use reasonable time**: 100-300 ms for most motions
3. **Reset before motion**: Always call `reset()` before starting
4. **Test on real field**: Friction and battery voltage affect settling
5. **Monitor oscillation**: If robot overshoots, increase time or reduce PID gains
6. **Use timeout safety**: Always have a maximum timeout to prevent stalling
7. **Tune PID first**: Settle conditions based on PID performance
8. **Document values**: Save your tuned exit conditions for reference

## See Also

- [PID Controller](./pid.md) - Works with exit conditions to control motion
- [Motion Algorithms](./motions.md) - Uses exit conditions to determine completion
- [Configuration](../tutorials/2_configuration.md) - Setting up exit conditions
- [Page System](./pages.md) - Real-time tuning UI for exit conditions
