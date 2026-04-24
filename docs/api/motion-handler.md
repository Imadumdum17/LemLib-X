# Motion Handler Documentation

## Overview

The MotionHandler is a task scheduler that manages motion execution. It runs motions in background tasks while allowing your main code to continue, but enforces that only one motion executes at a time.

## Motivation

Without the MotionHandler, running multiple motions would be complex:
```cpp
// Without MotionHandler - problematic:
moveToPoint(target1);      // Blocks until complete
moveToPoint(target2);      // Later code waits

// With MotionHandler - clean:
lemlib::motion_handler::move([&] { moveToPoint(target1); });  // Non-blocking
std::cout << "Motion started!" << std::endl;  // Executes immediately
lemlib::motion_handler::move([&] { moveToPoint(target2); });  // Waits for first to finish
```

## Key Concepts

### Non-Blocking Execution
Your code continues while the motion runs in a background task.

### Serialized Motions
Only one motion runs at a time. New motion requests wait for the current motion to finish.

### Task Notification Protocol
Motions detect cancellation via PROS task notification mechanism.

## API Reference

### `void move(std::function<void(void)> f)`

Schedules a motion function to run in the background.

**Parameters:**
- `f`: Function/lambda that contains the motion logic

**Behavior:**
- If no motion is running: Starts immediately in background
- If motion is running: Blocks until current motion finishes, then starts new one
- The calling code continues immediately (non-blocking)

**Example:**
```cpp
// Define motion
void goForward() {
    // Motion logic - must check for cancellation
    while (!pros::Task::notify_take(true, 0)) {
        // Update odometry, run PID, etc.
        pros::delay(10);
    }
}

// Schedule motion in autonomous
void autonomous() {
    lemlib::motion_handler::move(goForward);
    std::cout << "Motion started!" << std::endl;
    
    // Code here runs while motion executes
    pros::delay(100);
    
    // Start another motion - waits for first to finish
    lemlib::motion_handler::move(goForward);
}
```

### `bool isMoving()`

Checks if a motion is currently running.

**Returns:** `true` if a motion task exists and is active

**Example:**
```cpp
lemlib::motion_handler::move(longMotion);

if (lemlib::motion_handler::isMoving()) {
    std::cout << "Motion is running" << std::endl;
}

pros::delay(100);

if (lemlib::motion_handler::isMoving()) {
    std::cout << "Motion still running" << std::endl;
}
```

### `bool cancel()`

Cancels the currently running motion (if any).

**Returns:** `true` if a motion was cancelled

**Behavior:**
- Sends task notification to the motion task
- Motion must respect the notification (check in its loop)
- Cancellation is immediate - motion stops checking loop and exits
- Subsequent `move()` calls can start new motions

**Example:**
```cpp
lemlib::motion_handler::move(longMotion);
pros::delay(500);

// Cancel it
if (lemlib::motion_handler::cancel()) {
    std::cout << "Motion was cancelled" << std::endl;
}

pros::delay(100);

// Motor state won't change - motion is stopped
// Safe to start new motion
lemlib::motion_handler::move(newMotion);
```

## Motion Cancellation Protocol

For a motion to be cancellable, its loop must check for task notification:

```cpp
void cancellablMotion() {
    std::uint32_t prevTime = pros::millis();
    
    // Loop continues until task is notified
    while (!pros::Task::notify_take(true, 0)) {
        // Motion logic here
        
        // Update motors, sensors, etc.
        updateOdometry();
        calculatePID();
        applyMotorCommands();
        
        // Standard PROS delay pattern (prevents stale-checking)
        pros::Task::delay_until(&prevTime, 10);  // 10ms loop
    }
    
    // Motion exits when notified
    // Clean up here if needed
    leftMotors.moveVoltage(0);
    rightMotors.moveVoltage(0);
}
```

**Key points:**
- `pros::Task::notify_take(true, 0)`: Returns true when task is notified
  - First param `true`: Clear the notification
  - Second param `0`: Don't block (return immediately)
- Call this regularly (typically every 10 ms)
- When cancellation arrives, loop exits and motion cleans up

## Usage Patterns

### Basic Motion Sequence

```cpp
void autonomous() {
    // Wait for first motion to complete before starting second
    lemlib::motion_handler::move(moveToPoint(0, 0));      // Move 1
    lemlib::motion_handler::move(turnTo(90_deg));         // Move 2 (waits for Move 1)
    lemlib::motion_handler::move(moveToPoint(24, 0));     // Move 3 (waits for Move 2)
}
```

### Parallel Setup + Motion

```cpp
void autonomous() {
    // Start motion in background
    lemlib::motion_handler::move([&] {
        moveToPoint(24, 24);
    });
    
    // Do other stuff while moving
    arm.moveToHeight(50);
    intake.start();
    
    // Wait for motion if needed
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    // Now motion is done
    std::cout << "Movement complete!" << std::endl;
}
```

### Cancellation Handler

```cpp
void autonomous() {
    lemlib::motion_handler::move([&] {
        moveToPoint(200, 200);  // Far away
    });
    
    // Check for timeout or other condition
    pros::delay(2000);  // 2 second limit
    
    if (lemlib::motion_handler::isMoving()) {
        lemlib::motion_handler::cancel();
        std::cout << "Motion cancelled due to timeout" << std::endl;
    }
    
    // Start recovery motion
    lemlib::motion_handler::move([&] {
        moveToPoint(10, 10);  // Shorter distance
    });
}
```

### Conditional Chains

```cpp
void autonomous() {
    lemlib::motion_handler::move([&] {
        moveToPoint(20, 0);
    });
    
    // Wait for completion
    while (lemlib::motion_handler::isMoving()) {
        pros::delay(10);
    }
    
    // Check outcome and branch
    if (robot.isClear()) {
        lemlib::motion_handler::move([&] { moveToPoint(40, 0); });
    } else {
        lemlib::motion_handler::move([&] { turnTo(45_deg); });
    }
}
```

### Lambda Functions

```cpp
// Anonymous motion using lambda
lemlib::motion_handler::move([&] {
    // Capture everything by reference (&)
    while (!pros::Task::notify_take(true, 0)) {
        // Custom motion logic
        Number error = target - getCurrentPos();
        motor.moveVoltage(pid.update(error));
        
        pros::Task::delay_until(&prevTime, 10);
    }
});
```

## Integration with LemLib Motion Algorithms

LemLib's built-in motions already use the MotionHandler internally:

```cpp
// These are already wrapped with motion_handler::move()
lemlib::moveToPoint(target, timeout);           // Non-blocking
lemlib::moveToPose(target, heading, timeout);   // Non-blocking
lemlib::turnTo(heading, timeout);               // Non-blocking
lemlib::follow(path, lookahead, timeout);       // Non-blocking

// So code like this works naturally:
void autonomous() {
    moveToPoint(Point(0, 0), 5000_msec);
    
    // Subsequent motion waits for this to complete
    turnTo(90_deg, 2000_msec);
}
```

## Error Handling

### Motion Doesn't Cancel

```cpp
void autonomous() {
    lemlib::motion_handler::move(motion);
    
    // Request cancellation
    bool wasCancelled = lemlib::motion_handler::cancel();
    
    if (!wasCancelled) {
        std::cout << "No motion was running" << std::endl;
    }
}
```

### Motion Hangs (Never Completes)

```cpp
// Motion must exit - infinite loops cause stalling
void badMotion() {
    while (true) {  // NEVER EXITS!
        // motion logic
    }
}

// Correct pattern - always has exit condition
void goodMotion() {
    while (!pros::Task::notify_take(true, 0)) {  // EXIT WHEN NOTIFIED
        // motion logic
        if (settledCondition) break;  // OR WHEN SETTLED
    }
}
```

## Performance Considerations

### Stack Size

Motion tasks need sufficient stack for their local variables:

```cpp
// Large local arrays need more stack
void motionWithBigBuffer() {
    std::array<Number, 1000> buffer;  // Allocates on stack
    // Ensure task has enough stack
}
```

### Loop Frequency

Keep motion loop update rate consistent:

```cpp
// Good - consistent 10ms updates
std::uint32_t prevTime = pros::millis();
while (!pros::Task::notify_take(true, 0)) {
    // 10ms motion logic
    pros::Task::delay_until(&prevTime, 10);  // Maintains ~100Hz
}

// Bad - variable timing
while (!pros::Task::notify_take(true, 0)) {
    // Variable delay causes jitter
    pros::delay(10);  // Not guaranteed timing
}
```

## Best Practices

1. **Always check for notification** in motion loops
2. **Use lambda functions** to capture context naturally
3. **Clean up motors** when motion ends (set voltage to 0)
4. **Set reasonable timeouts** in motion algorithms
5. **Use `isMoving()`** to synchronize motion sequences
6. **Test cancellation** - ensure motions exit cleanly
7. **Monitor stack usage** for complex motions
8. **Document motion behavior** for team reference

## Advanced Customization

### Custom Motion with Full Features

```cpp
void customMotion(Point target) {
    // Setup
    auto startTime = pros::millis();
    auto startPose = getPose();
    pid.reset();
    exit.reset();
    
    std::uint32_t prevTime = pros::millis();
    
    // Motion loop - respects cancellation
    while (!pros::Task::notify_take(true, 0)) {
        // Check timeout
        if ((pros::millis() - startTime) > MOTION_TIMEOUT) break;
        
        // Update control
        Number error = target.distance(getPose());
        Number output = pid.update(error);
        
        // Apply movement
        leftMotors.moveVoltage(output);
        rightMotors.moveVoltage(output);
        
        // Check settling
        if (exit.update(error)) break;
        
        // Consistent loop timing
        pros::Task::delay_until(&prevTime, 10);
    }
    
    // Cleanup
    leftMotors.moveVoltage(0);
    rightMotors.moveVoltage(0);
}

// Usage
void autonomous() {
    lemlib::motion_handler::move([&] {
        customMotion(Point(24, 24));
    });
}
```

## See Also

- [Motion Algorithms](./motions.md) - How moveToPoint, follow, etc. work
- [PID Controller](./pid.md) - Error correction used in motions
- [ExitCondition](./exit-condition.md) - Determines when motion ends
- [PROS Task Documentation](https://pros.cs.purdue.edu/v5/api/cpp/rtos.html)
