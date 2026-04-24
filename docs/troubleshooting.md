# Troubleshooting Guide

## Common Issues and Solutions

This guide covers problems you might encounter when using LemLib-X and how to solve them.

## Motion Issues

### Problem: Robot Doesn't Move at All

**Symptoms:**
- Motor is silent
- No robot movement
- Tests in opcontrol work fine

**Diagnosis Steps:**

1. **Check motor ports:**
   ```cpp
   // Verify ports are 1-21
   lemlib::V5Motor motor(1);  // Valid
   lemlib::V5Motor motor(25); // INVALID - causes crash
   ```

2. **Test motors in opcontrol:**
   ```cpp
   void opcontrol() {
       left_motors.moveVoltage(8000);   // Should move
       pros::delay(1000);
       left_motors.moveVoltage(0);      // Stop
   }
   ```

3. **Check physical connections:**
   - Verify motor cables are seated
   - Ensure motor is not disabled
   - Check for broken wires

**Solution:**
- Correct port numbers in motor definitions
- Verify motor power is not locked/disabled
- Test individual motors before grouping

---

### Problem: Robot Moves Wrong Direction

**Symptoms:**
- Forward command makes robot go backward
- Left and right sides move opposite directions
- Motion goes opposite of intended

**Diagnosis Steps:**

1. **Check reversal flags:**
   ```cpp
   // Current (wrong) setup
   lemlib::V5Motor rightMotor(3);  // NOT reversed
   
   // What happens:
   // left command +100V → robot goes left
   // right command +100V → robot goes right (WRONG - should go right too)
   ```

2. **Test which side is wrong:**
   ```cpp
   void opcontrol() {
       // Try each side separately
       left_motors.moveVoltage(8000);   // Push forward?
       pros::delay(500);
       left_motors.moveVoltage(0);
       
       right_motors.moveVoltage(8000);  // Push forward too?
       pros::delay(500);
       right_motors.moveVoltage(0);
   }
   ```

**Solution:**
- Add `true` to reverse motors that spin opposite direction:
  ```cpp
  lemlib::V5Motor front_right(3, true);  // Reverse this one
  lemlib::V5Motor back_right(4, true);   // And this one
  ```

---

### Problem: Motion Completes Too Quickly (or Too Slowly)

**Symptoms:**
- Motion ends before reaching target
- Motion takes forever to complete
- Inconsistent timing between runs

**Root Causes:**
- PID gains too high (overshooot) or too low (sluggish)
- Exit conditions too lenient or too strict
- Exit condition timeout too short

**PID-Based Solutions:**

```cpp
// Problem: Moves too fast and overshoots
// Solution: Reduce kP, increase kD
lemlib::PID lateral_pid(0.5, 0.02, 0.08);  // Lower response, more damping

// Problem: Moves too slowly, takes forever
// Solution: Increase kP, reduce kI
lemlib::PID lateral_pid(1.2, 0.02, 0.08);  // Higher response
```

**Exit Condition Solutions:**

```cpp
// Problem: Motion ends too early (before reaching target)
lemlib::ExitCondition<Length> exit(5_in, 100_msec);
// Solution: Tighten range or increase time
lemlib::ExitCondition<Length> exit(2_in, 250_msec);

// Problem: Motion waits forever at target
// Solution: Increase range (target is hard to achieve perfectly)
lemlib::ExitCondition<Length> exit(3_in, 200_msec);
```

---

### Problem: Motion Oscillates Around Target

**Symptoms:**
- Robot jumps left-right while approaching target
- Oscillation becomes more pronounced as it settles
- Motor commands rapidly switch sign

**Root Cause:**
- PID gains too aggressive
- Insufficient damping (low or zero kD)
- Exit condition time too short

**Solutions:**

1. **Increase Damping (kD):**
   ```cpp
   // Before: Oscillates
   lemlib::PID lateral_pid(0.8, 0.05, 0.05);
   
   // After: Damped oscillation
   lemlib::PID lateral_pid(0.8, 0.05, 0.15);
   ```

2. **Reduce Proportional Gain (kP):**
   ```cpp
   // Reduce responsiveness
   lemlib::PID lateral_pid(0.5, 0.05, 0.10);
   ```

3. **Increase Exit Condition Time:**
   ```cpp
   // Give system time to settle
   lemlib::ExitCondition<Length>(2_in, 300_msec);  // Was 150_msec
   ```

---

### Problem: Motion Never Completes (Timeout Always Reached)

**Symptoms:**
- Motion always runs for full timeout period
- Exit condition never triggered
- Motion never reaches target

**Root Causes:**
- Exit condition range unrealistic
- Robot can't physically reach target
- PID gains are zero
- Exit condition time is infinite

**Diagnosis:**

```cpp
void diagnostic() {
    // Monitor error as robot moves
    while (motion_running) {
        auto error = target - current_position;
        printf("Error: %.2f inches\n", error);
        pros::delay(100);
    }
    
    // Check what minimum error is achieved
}
```

**Solutions:**

1. **Verify exit condition is achievable:**
   ```cpp
   // If robot only gets within 2 inches:
   lemlib::ExitCondition<Length> exit(1_in, 200_msec);  // Won't work!
   
   // Use achievable range:
   lemlib::ExitCondition<Length> exit(3_in, 200_msec);  // Better
   ```

2. **Check PID gains aren't zero:**
   ```cpp
   // Make sure this isn't the config:
   lemlib::PID pid(0, 0, 0);  // Won't move!
   
   // Should have at least proportional:
   lemlib::PID pid(0.5, 0, 0);
   ```

3. **Verify robot can reach the target:**
   - Test moving motors manually
   - Check for wheel slip
   - Verify no mechanical blockage

---

## Odometry Issues

### Problem: Position is Always Wrong

**Symptoms:**
- Robot position doesn't match actual location
- Numbers are completely off
- Odometry is zeroed but wrong immediately

**Diagnosis Steps:**

1. **Test encoder readings:**
   ```cpp
   void test_encoders() {
       // Start at origin
       odom.setPose(Pose(0, 0, 0_deg));
       
       // Drive forward 12 inches manually
       // Measured distance: move left motor exactly 12 inches
       
       auto pose = odom.getPose();
       printf("Position: (%.1f, %.1f, %.1f)\n",
              pose.x.convert(inch),
              pose.y.convert(inch),
              pose.theta.convert(degree));
       
       // Check: Should report ~(12, 0, 0)
       // If different, encoders are miscalibrated
   }
   ```

2. **Check encoder connections:**
   - Verify ADI ports are correct ('A', 'B' not ('A', 'A'))
   - Check cables are fully inserted
   - Verify no bent pins

3. **Verify coordinate system:**
   - Robot at (0, 0) facing 0°
   - Drive forward
   - Should report Y increasing, not X

**Solutions:**

1. **Recalibrate encoders:**
   - Mark start position
   - Drive exact known distance
   - Adjust calibration factors

2. **Fix port assignments:**
   ```cpp
   // Wrong:
   lemlib::ADIEncoder enc('A', 'A');  // Same port twice!
   
   // Correct:
   lemlib::ADIEncoder enc('A', 'B');  // Different ports
   ```

---

### Problem: Robot Drifts Off Course While Moving

**Symptoms:**
- Over time, reported position becomes inaccurate
- Heading drift accumulates
- Odometry error growing continuously

**Root Causes:**
- IMU not calibrated
- Tracking wheel placement incorrect
- Drift compensation too low
- Encoder slip

**Solutions:**

1. **Recalibrate IMU:**
   ```cpp
   void initialize() {
       imu.calibrate();  // Must happen at startup
       while (imu.isCalibrating()) {
           pros::delay(50);
           // DO NOT ROTATE ROBOT!
       }
   }
   ```

2. **Verify tracking wheel placement:**
   - Horizontal wheel perpendicular to drive
   - Vertical wheel parallel to drive
   - Both centered on robot
   - Not touching floor

3. **Check drift compensation:**
   ```cpp
   // Increase trust in IMU
   const Number drift_compensation = 0.7;  // Was 0.5
   
   // Or trust tracking wheels more:
   const Number drift_compensation = 0.3;
   ```

---

### Problem: IMU Calibration Fails

**Symptoms:**
- Calibration takes forever
- "IMU Calibrating" message never disappears
- IMU heading is garbage values

**Root Causes:**
- Port number is wrong
- IMU is upside down
- Robot was rotated during calibration
- Faulty IMU hardware

**Diagnosis:**

```cpp
void test_imu() {
    printf("Starting IMU...\n");
    pros::Imu imu(port);  // Test with low-level PROS IMU
    
    printf("Head: %.1f\n", imu.get_heading());
    // If 0 but should be different, IMU might be upside down
    
    pros::delay(1000);
    printf("Head: %.1f\n", imu.get_heading());
    // If same as before, calibration stuck
}
```

**Solutions:**

1. **Verify port number:**
   ```cpp
   // Change port (must be 1-21)
   lemlib::V5InertialSensor imu(11);  // Try different port
   ```

2. **Keep robot still during calibration:**
   ```cpp
   printf("Calibrating IMU - DO NOT MOVE ROBOT\n");
   imu.calibrate();
   
   // Wait with robot completely still
   while (imu.isCalibrating()) {
       pros::delay(50);
   }
   ```

3. **Check physical orientation:**
   - Mount IMU level on robot
   - Make sure it's not upside down
   - Verify no loose mounting

---

## Configuration Issues

### Problem: "Undefined Reference" Compile Error

**Error Message:**
```
undefined reference to `lateral_pid'
undefined reference to `left_motors'
```

**Root Cause:**
- Declared in `config.hpp` but not implemented
- Forgot to include `config.hpp` in main.cpp

**Solution:**

1. **Add implementations to main.cpp:**
   ```cpp
   // Include config declarations
   #include "lemlib/config.hpp"
   
   // Implement them:
   lemlib::MotorGroup left_motors({&leftMotor1, &leftMotor2});
   lemlib::MotorGroup right_motors({&rightMotor1, &rightMotor2});
   
   lemlib::PID lateral_pid(0.8, 0.05, 0.1);
   lemlib::PID angular_pid(1.0, 0.1, 0.15);
   
   // etc...
   ```

---

### Problem: PID Changes Don't Take Effect

**Symptoms:**
- Changed PID values but motion behaves same
- Recompile doesn't help
- Changes only work if I use setAngularPID()

**Root Cause:**
- Motion algorithms read config at startup
- Your changes didn't recompile
- Build cache not cleared

**Solutions:**

1. **Force PID update explicitly:**
   ```cpp
   void autonomous() {
       // Force new gains
       lemlib::Gains gains = {0.9, 0.05, 0.12};
       setLateralPID(lemlib::PID(gains));
       
       lemlib::moveToPoint(target, timeout);
   }
   ```

2. **Clean build:**
   ```bash
   pros build --clean
   pros upload
   ```

3. **Verify compilation:**
   ```bash
   pros build --verbose
   
   # Look for your PID initialization in output
   ```

---

## Hardware Issues

### Problem: Motor Makes Weird Noise Then Stops

**Symptoms:**
- Motor squeals or grinds
- Motor stops after brief movement
- Code crashes or hangs

**Root Causes:**
- Motor overheating
- Excessive current (mechanical jam)
- Motor firmware error
- Reversed polarity (shouldn't happen via software)

**Solutions:**

1. **Check for mechanical issues:**
   - Remove wheels and spin drive freely
   - Look for binding gears
   - Verify no broken bearings

2. **Reduce current demand:**
   ```cpp
   // Less aggressive acceleration:
   const Number lateral_slew = 300;  // Was 600
   ```

3. **Reset motor firmware:**
   - Plug motor into V5 Brain directly
   - Use PROS commands to reset
   - Try different motor if available

---

### Problem: Encoder Always Reports Zero

**Symptoms:**
- Encoder position never changes
- Always reads 0.0
- Motor moves but encoder doesn't register

**Root Causes:**
- Encoder not plugged in
- Wrong ADI ports
- ADI expander not configured
- Encoder is broken

**Diagnosis:**

```cpp
void test_encoder() {
    lemlib::ADIEncoder enc('A', 'B');
    
    printf("Position: %f\n", enc.getPosition());
    
    // Manually spin wheel
    
    printf("Position: %f\n", enc.getPosition());
    
    // Should show change if working
}
```

**Solutions:**

1. **Verify ADI connection:**
   - Check ports ('A', 'B'), not ('A', 'A')
   - Test with different ports
   - Ensure ADI expander is installed

2. **Test with V5 Rotation Sensor instead:**
   ```cpp
   // If using ADI encoder fails:
   lemlib::V5RotationSensor enc(11);  // Use V5 sensor
   ```

---

## Performance Issues

### Problem: Motion is Jerky or Jittery

**Symptoms:**
- Robot movement stutters
- Motion appears choppy
- Inconsistent acceleration

**Root Causes:**
- Loop update rate too slow
- Sensor readings noisy
- PID gains cause rapid oscillations
- CPU overloaded

**Solutions:**

1. **Reduce other tasks:**
   - Disable unnecessary background tasks
   - Minimize logging
   - Simplify vision processing

2. **Smooth PID response:**
   ```cpp
   // Reduce responsiveness:
   lemlib::PID pid(0.6, 0.03, 0.12);  // Lower gains
   ```

3. **Verify loop timing:**
   ```cpp
   // Check update frequency
   std::uint32_t prevTime = pros::millis();
   while (true) {
       printf("Loop time: %u ms\n", pros::millis() - prevTime);
       prevTime = pros::millis();
       
       // Should be ~10ms
   }
   ```

---

### Problem: Battery Voltage Affects Motion Quality

**Symptoms:**
- Motion works perfectly with fresh battery
- Same motion barely works with discharged battery
- Tuning values don't work consistently

**Root Causes:**
- Motor voltage dependent on battery
- PID gains tuned for one battery level
- Exit conditions too tight for low voltage

**Solutions:**

1. **Normalize motor control:**
   ```cpp
   // Use battery_volts (PROS built-in)
   motor.moveVoltage(output * 12000 / pros::battery_get_voltage());
   ```

2. **Tune for low voltage:**
   - Test with ~50% charge
   - Use more aggressive gains
   - Increase timeouts

3. **Monitor and adjust:**
   ```cpp
   float battery = pros::battery_get_voltage() / 1000.0;
   printf("Battery: %.1fV\n", battery);
   ```

---

## Testing Checklist

Use this checklist when debugging issues:

- [ ] **Motors**
  - [ ] Can move all motors individually
  - [ ] All motors respond in opcontrol
  - [ ] No grinding or unusual noises

- [ ] **Sensors**
  - [ ] IMU reports reasonable headings
  - [ ] Encoders register movement
  - [ ] Tracking wheels track correctly

- [ ] **Configuration**
  - [ ] Motor ports are valid (1-21)
  - [ ] Reverse flags correct
  - [ ] Config compiles without errors

- [ ] **Odometry**
  - [ ] Position updates when moving
  - [ ] Heading matches physical orientation
  - [ ] Resets correctly with setPose

- [ ] **PID**
  - [ ] Motors respond to PID output
  - [ ] Gains are non-zero
  - [ ] Exit conditions are achievable

- [ ] **Motion**
  - [ ] Simple moveToPoint works
  - [ ] Motion completes (or times out)
  - [ ] Cannot move during motion

---

## Getting Help

If you're still stuck:

1. **Check the docs:** Review relevant section in [API Reference](./api/index.md)
2. **Enable debug output:** Print values to diagnose
3. **Test incrementally:** Isolate problem to one component
4. **Ask community:** LemLib Discord or RECF forums
5. **Report bug:** GitHub issues if reproducible

See [Support](./support.md) for additional resources.
