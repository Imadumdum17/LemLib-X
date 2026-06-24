# 1 - Getting Started

This tutorial gets a basic LemLib-X drivetrain moving. By the end, you should have:

- left and right drive motor groups
- an IMU and tracking wheel odometry
- default motion settings
- one simple autonomous movement

The examples use the `ll` and `mh` aliases from `lemlib/lemlib.hpp`. `ll` means `lemlib_x`, and `mh` means `lemlib_x::motion_handler`.

## Create the drivetrain

Start by including LemLib-X and declaring your hardware near the top of `src/main.cpp`.

```cpp
#include "main.h"
#include "lemlib/lemlib.hpp"
#include "pros/llemu.hpp"

ll::MotorGroup left_motors({1, 2, 3}, 360_rpm);
ll::MotorGroup right_motors({-4, -5, -6}, 360_rpm);

ll::V5InertialSensor imu(7);

ll::TrackingWheel vertical_wheel(8, 2.75_in, 0_in);
ll::TrackingWheel horizontal_wheel(9, 2.75_in, 0_in);

ll::TrackingWheelOdometry odom({&imu}, {&vertical_wheel}, {&horizontal_wheel});
```

Negative motor ports reverse that motor. If the robot drives backward when both sides are given positive power, reverse the ports on one side.

## Add default motion configuration

The motion algorithms look for the global values declared in `include/lemlib/config.hpp`. Put these definitions in `src/main.cpp` after your hardware objects.

```cpp
ll::PID lateral_pid(0.8, 0, 0.05);
ll::PID angular_pid(1.2, 0, 0.08);

const std::function<units::Pose()> pose_getter = [] {
    return odom.getPose();
};

const Length track_width = 11.5_in;
const Number drift_compensation = 0.6;
const Number lateral_slew = 8;
const Number angular_slew = 8;

const ll::ExitConditionGroup<Length> lateral_exit_conditions({
    ll::ExitCondition<Length>(1_in, 250_msec),
    ll::ExitCondition<Length>(3_in, 750_msec),
});

const ll::ExitConditionGroup<AngleRange> angular_exit_conditions({
    ll::ExitCondition<AngleRange>(2_stDeg, 250_msec),
    ll::ExitCondition<AngleRange>(5_stDeg, 750_msec),
});

void setLateralPID(const ll::PID& pid) {
    lateral_pid = pid;
}

void setAngularPID(const ll::PID& pid) {
    angular_pid = pid;
}
```

These numbers are only a safe starting point. Expect to tune them for your robot.

## Initialize sensors

Calibrate the IMU before starting odometry.

```cpp
void initialize() {
    pros::lcd::initialize();

    imu.calibrate();
    pros::delay(3000);

    odom.setPose({0_in, 0_in, 0_cDeg});
    odom.startTask();

    pros::lcd::print(0, "LemLib-X ready");
}
```

If your IMU takes longer than 3 seconds to calibrate, increase the delay or wait until your own sensor wrapper reports that calibration is finished.

## Test driver control first

Before running autonomous, make sure the drivetrain responds correctly.

```cpp
void opcontrol() {
    pros::Controller controller(pros::E_CONTROLLER_MASTER);

    while (true) {
        const double left = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0;
        const double right = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y) / 127.0;

        left_motors.move(left);
        right_motors.move(right);

        const auto pose = odom.getPose();
        pros::lcd::print(0, "X: %.1f", to_in(pose.x));
        pros::lcd::print(1, "Y: %.1f", to_in(pose.y));
        pros::lcd::print(2, "H: %.1f", to_cDeg(pose.orientation));

        pros::delay(10);
    }
}
```

Push both sticks forward. The robot should drive forward, and the odometry display should change smoothly.

## Run the first motion

Use `moveToPoint` for the first autonomous test.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }

    pros::lcd::print(0, "Motion complete");
}
```

Keep the first target short. Once 24 inches is repeatable, move on to configuration and tuning.

## Checklist

- The robot drives forward when both motor groups receive positive power.
- The IMU is still while calibrating.
- Odometry changes in the expected direction when the robot is pushed by hand.
- `moveToPoint` stops before the timeout.
- The robot brakes at the end of the motion.

Next: [Configuration](./2_configuration.md)
