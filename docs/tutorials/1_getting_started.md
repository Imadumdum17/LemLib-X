# 1 - Getting Started

This tutorial walks through the smallest useful LemLib-X setup: a differential drivetrain, an IMU, tracking wheel odometry, and one autonomous motion.

By the end, you should be able to:

- create left and right drive motor groups
- create tracking wheel odometry
- expose the default configuration values used by motions
- display pose feedback on the brain screen
- run a short `moveToPoint` test

The examples use the aliases from `lemlib/lemlib.hpp`:

- `ll` is `lemlib_x`
- `mh` is `lemlib_x::motion_handler`

## Before You Start

You need a PROS V5 project with LemLib-X installed and a drivetrain that can drive as a normal tank drive. Do not tune autonomous before basic driver control works.

Have these measurements ready:

| Measurement | What it means |
| --- | --- |
| Drive motor ports | Negative ports reverse motors |
| Drive output speed | Wheel output speed after gearing, such as `360_rpm` |
| Tracking wheel diameter | Common values are `2_in` or `2.75_in` |
| Tracking wheel offset | Distance from robot tracking center to each wheel |
| IMU port | Smart port used by the inertial sensor |

The examples assume:

- positive X is forward from the starting pose
- positive heading turns counterclockwise
- distances are written in inches
- headings are written in centidegrees with `_cDeg`

## Create the Drivetrain

Put your hardware declarations near the top of `src/main.cpp`.

```cpp
#include "main.h"
#include "hardware/IMU/V5InertialSensor.hpp"
#include "lemlib/lemlib.hpp"
#include "pros/llemu.hpp"
#include <vector>

ll::MotorGroup left_motors({1, 2, 3}, 360_rpm);
ll::MotorGroup right_motors({-4, -5, -6}, 360_rpm);

ll::V5InertialSensor imu(7);

ll::TrackingWheel vertical_wheel(8, 2.75_in, 0_in);
ll::TrackingWheel horizontal_wheel(9, 2.75_in, 0_in);

ll::TrackingWheelOdometry odom(
    std::vector<ll::IMU*>{&imu},
    std::vector<ll::TrackingWheel*>{&vertical_wheel},
    std::vector<ll::TrackingWheel*>{&horizontal_wheel}
);
```

Use `ll::V5InertialSensor`, not `pros::Imu`. `TrackingWheelOdometry` stores pointers to LemLib-X's `IMU` interface, so a raw PROS IMU pointer will not match the constructor.

Negative motor ports reverse that motor. If pushing both joysticks forward makes one side drive backward, reverse that side's ports.

## Add Default Motion Configuration

The motion algorithms use the global values declared in `include/lemlib/config.hpp`. Define them once in `src/main.cpp` after your hardware objects.

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

These numbers are starting values only. A different robot may need very different gains.

## Initialize Sensors

Calibrate the IMU before starting odometry. Keep the robot still during calibration.

```cpp
void initialize() {
    pros::lcd::initialize();

    imu.calibrate();
    while (imu.isCalibrating()) {
        pros::delay(20);
    }

    odom.setPose({0_in, 0_in, 0_cDeg});
    odom.startTask();

    pros::lcd::print(0, "LemLib-X ready");
}
```

If your IMU wrapper reports calibration inconsistently, use a fixed delay such as `pros::delay(3000)` and test again.

## Add a Pose Display

Pose feedback makes setup problems obvious. Add this helper:

```cpp
void printPose() {
    const auto pose = odom.getPose();
    pros::lcd::print(0, "X: %.1f", to_in(pose.x));
    pros::lcd::print(1, "Y: %.1f", to_in(pose.y));
    pros::lcd::print(2, "H: %.1f", to_cDeg(pose.orientation));
}
```

You can call it in `opcontrol()` or in a background task.

```cpp
pros::Task screen_task([] {
    while (true) {
        printPose();
        pros::delay(50);
    }
});
```

## Test Driver Control First

Before autonomous, confirm the drivetrain responds correctly.

```cpp
void opcontrol() {
    pros::Controller controller(pros::E_CONTROLLER_MASTER);

    while (true) {
        const double left = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0;
        const double right = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y) / 127.0;

        left_motors.move(left);
        right_motors.move(right);
        printPose();

        pros::delay(10);
    }
}
```

Run these quick checks:

1. Push both sticks forward. The robot should drive forward.
2. Push only the left stick. The left side should move.
3. Push only the right stick. The right side should move.
4. Push the robot forward by hand. Pose should change smoothly.
5. Rotate the robot by hand. Heading should change smoothly.

## Run the First Motion

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

Keep the first target short. Do not start with a full-field motion; short motions are easier to diagnose.

## First Run Troubleshooting

| Problem | Likely cause | What to check |
| --- | --- | --- |
| Robot drives backward | Motor reversal wrong | Make one side's motor ports negative |
| Robot spins instead of driving | One side is reversed incorrectly | Test each side in driver control |
| Pose changes while robot is still | IMU still calibrating or noisy sensor | Keep robot still, check sensor connection |
| Motion never starts | Missing config globals | Confirm every `extern` in `config.hpp` is defined once |
| Motion starts but never finishes | Exit conditions too strict or odometry wrong | Print pose and increase timeout |

## Ready to Continue

Move on when:

- driver control works
- odometry changes in the expected direction
- `moveToPoint({24_in, 0_in}, ...)` moves roughly forward
- the motion stops before the timeout

Next: [Configuration](./2_configuration.md)
