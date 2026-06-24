# 2 - Configuration

LemLib-X keeps robot-specific defaults in one place. Motion functions can use these defaults through their settings structs, so your autonomous code stays focused on targets instead of wiring.

## Required globals

`include/lemlib/config.hpp` declares the values that motion algorithms use by default:

```cpp
extern lemlib_x::MotorGroup left_motors;
extern lemlib_x::MotorGroup right_motors;
extern lemlib_x::PID lateral_pid;
extern lemlib_x::PID angular_pid;
extern const std::function<units::Pose()> pose_getter;
extern const lemlib_x::ExitConditionGroup<Length> lateral_exit_conditions;
extern const lemlib_x::ExitConditionGroup<AngleRange> angular_exit_conditions;
extern const Length track_width;
extern const Number drift_compensation;
extern const Number lateral_slew;
extern const Number angular_slew;
```

Define each of them once in a `.cpp` file, usually `src/main.cpp`.

## Motor groups

Use one group for each side of a differential drivetrain.

```cpp
ll::MotorGroup left_motors({1, 2, 3}, 360_rpm);
ll::MotorGroup right_motors({-4, -5, -6}, 360_rpm);
```

The second argument is the theoretical output speed after gearing. Use the speed of your drive output, not necessarily the motor cartridge speed.

## Odometry and pose getter

The pose getter is how motions know where the robot is.

```cpp
ll::V5InertialSensor imu(7);
ll::TrackingWheel vertical_wheel(8, 2.75_in, 0_in);
ll::TrackingWheel horizontal_wheel(9, 2.75_in, 0_in);

ll::TrackingWheelOdometry odom({&imu}, {&vertical_wheel}, {&horizontal_wheel});

const std::function<units::Pose()> pose_getter = [] {
    return odom.getPose();
};
```

Tracking wheel offset is the distance from the robot's tracking center to the wheel. Positive and negative direction depend on where the wheel sits relative to the center, so verify the sign with a simple push test.

## PID controllers

Use separate controllers for forward movement and turning.

```cpp
ll::PID lateral_pid(0.8, 0, 0.05);
ll::PID angular_pid(1.2, 0, 0.08);
```

Start with only `kP` and a small `kD`. Add `kI` only after the robot is stable and still has a consistent final error.

## Exit conditions

Exit conditions decide when a motion is settled.

```cpp
const ll::ExitConditionGroup<Length> lateral_exit_conditions({
    ll::ExitCondition<Length>(1_in, 250_msec),
    ll::ExitCondition<Length>(3_in, 750_msec),
});

const ll::ExitConditionGroup<AngleRange> angular_exit_conditions({
    ll::ExitCondition<AngleRange>(2_stDeg, 250_msec),
    ll::ExitCondition<AngleRange>(5_stDeg, 750_msec),
});
```

The first condition is the normal precise exit. The second condition gives the robot a forgiving way to finish if it is close enough but not perfect.

## Slew and drift values

Slew limits how quickly motor output can change. Lower values are smoother; higher values are more aggressive.

```cpp
const Number lateral_slew = 8;
const Number angular_slew = 8;
```

`drift_compensation` is used by `moveToPose` to reduce sliding through curved approaches.

```cpp
const Number drift_compensation = 0.6;
```

If the robot cuts corners or slides during curved motion, lower the maximum speed first, then adjust drift compensation.

## Runtime setters

These setters let tools or test code update PID values without changing the rest of the config.

```cpp
void setLateralPID(const ll::PID& pid) {
    lateral_pid = pid;
}

void setAngularPID(const ll::PID& pid) {
    angular_pid = pid;
}
```

## Passing custom settings

Most code can use defaults:

```cpp
ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});
```

For a one-off test, override only the pieces you need.

```cpp
ll::PID test_lateral(0.6, 0, 0.03);

ll::moveToPoint(
    {24_in, 0_in},
    4_sec,
    {},
    {.lateralPID = test_lateral}
);
```

Next: [Driver Control](./3_driver_control.md)
