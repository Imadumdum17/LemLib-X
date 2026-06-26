# 2 - Configuration

LemLib-X uses a small set of global defaults for motion algorithms. Once these are defined, calls like `moveToPoint`, `moveToPose`, and `turnTo` can use your robot's drivetrain, odometry, PID gains, exit conditions, and slew settings automatically.

This tutorial explains what each default does, how to measure it, and how to override it for special cases.

## Required Globals

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

Define each of them once in a `.cpp` file, usually `src/main.cpp`. Multiple definitions will cause linker errors; missing definitions will cause unresolved symbols.

## Motor Groups

Use one motor group for each side of a differential drivetrain.

```cpp
ll::MotorGroup left_motors({1, 2, 3}, 360_rpm);
ll::MotorGroup right_motors({-4, -5, -6}, 360_rpm);
```

The first argument is the signed list of ports. A negative port reverses that motor. The second argument is the theoretical output speed of the drivetrain after gearing.

Examples:

| Setup | Output speed |
| --- | --- |
| Direct green cartridge drive | `200_rpm` |
| Direct blue cartridge drive | `600_rpm` |
| Geared blue drive with 3:5 reduction | `360_rpm` |

Use the speed at the wheel, not the motor cartridge speed, when gearing changes the output.

## Odometry

The pose getter is how motions know where the robot is.

```cpp
#include "hardware/IMU/V5InertialSensor.hpp"
#include <vector>

ll::V5InertialSensor imu(7);
ll::TrackingWheel vertical_wheel(8, 2.75_in, 0_in);
ll::TrackingWheel horizontal_wheel(9, 2.75_in, 0_in);

ll::TrackingWheelOdometry odom(
    std::vector<ll::IMU*>{&imu},
    std::vector<ll::TrackingWheel*>{&vertical_wheel},
    std::vector<ll::TrackingWheel*>{&horizontal_wheel}
);

const std::function<units::Pose()> pose_getter = [] {
    return odom.getPose();
};
```

The IMU must be a LemLib-X IMU wrapper. If you already have a `pros::Imu`, wrap it before passing it to odometry:

```cpp
ll::V5InertialSensor imu = ll::V5InertialSensor::from_pros_imu(pros::Imu(7));
```

## Tracking Wheel Placement

A vertical tracking wheel measures forward and backward movement. A horizontal tracking wheel measures sideways movement. Each tracking wheel has:

- an encoder or rotation sensor
- a wheel diameter
- an offset from the tracking center
- an optional gear ratio

```cpp
ll::TrackingWheel vertical_wheel(8, 2.75_in, 1.5_in);
ll::TrackingWheel horizontal_wheel(9, 2.75_in, -4.0_in);
```

Start with the measured offsets, then verify them by pushing the robot:

1. Set pose to `{0_in, 0_in, 0_cDeg}`.
2. Push forward exactly 24 inches.
3. Check that X changes by about 24 inches.
4. Push sideways if your drivetrain allows it, or rotate by hand.
5. If values move in the wrong direction, reverse the sensor or change the sign.

## PID Controllers

Use one PID for forward/backward error and another for heading error.

```cpp
ll::PID lateral_pid(0.8, 0, 0.05);
ll::PID angular_pid(1.2, 0, 0.08);
```

The parameters are:

| Term | Purpose | Start with |
| --- | --- | --- |
| `kP` | Main correction strength | Nonzero |
| `kI` | Helps remove small final error | `0` |
| `kD` | Dampens overshoot | Small value |

Keep `kI` at zero while first tuning. Integral can hide mechanical problems and make oscillation worse.

## Exit Conditions

Exit conditions decide when a motion is considered done.

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

Each condition means "the error must stay inside this range for this long." The group finishes when any condition finishes.

Use a strict condition for good runs and a looser fallback so the robot does not waste match time hunting for perfection.

## Slew Values

Slew limits how quickly motor output can change.

```cpp
const Number lateral_slew = 8;
const Number angular_slew = 8;
```

Lower slew values make starts smoother and reduce wheel slip. Higher values make the robot more aggressive.

If the robot jumps, skids, or lifts a wheel at the beginning of motion, reduce speed or use more conservative slew.

## Track Width and Drift Compensation

`track_width` is the effective drivetrain width used by path following.

```cpp
const Length track_width = 11.5_in;
```

Measure from the center of the left wheel contact patch to the center of the right wheel contact patch. If turns are consistently too wide or too tight, tune this value experimentally.

`drift_compensation` is used by `moveToPose` to limit speed through curved approaches.

```cpp
const Number drift_compensation = 0.6;
```

If the robot slides through arcs, lower max speed first. Then tune drift compensation if the path shape is still poor.

## Runtime Setters

These setters let pages or test code update PID values without replacing the config objects.

```cpp
void setLateralPID(const ll::PID& pid) {
    lateral_pid = pid;
}

void setAngularPID(const ll::PID& pid) {
    angular_pid = pid;
}
```

## Override Settings for One Motion

Most calls can use defaults:

```cpp
ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});
```

For one-off tuning, override just the setting you care about:

```cpp
ll::PID test_lateral(0.6, 0, 0.03);

ll::moveToPoint(
    {24_in, 0_in},
    4_sec,
    {},
    {.lateralPID = test_lateral}
);
```

This is useful when testing new gains without changing the global values used by the rest of autonomous.

## Configuration Checklist

- Motor ports match the robot.
- Reversed motors are negative.
- IMU is a LemLib-X IMU wrapper.
- Tracking wheel diameters are real measured diameters.
- Tracking wheel offsets have the correct sign.
- `pose_getter` returns `odom.getPose()`.
- PID values are conservative.
- Exit conditions have both a strict and forgiving condition.

Next: [Driver Control](./3_driver_control.md)
