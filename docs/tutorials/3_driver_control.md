# 3 - Driver Control

Autonomous tuning is much easier when driver control is boringly reliable. This tutorial adds a clean tank drive, brake mode setup, and a quick sensor display.

## Basic tank drive

Tank drive maps the left stick to the left motors and the right stick to the right motors.

```cpp
void opcontrol() {
    pros::Controller controller(pros::E_CONTROLLER_MASTER);

    while (true) {
        const double left = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0;
        const double right = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y) / 127.0;

        left_motors.move(left);
        right_motors.move(right);

        pros::delay(10);
    }
}
```

If one side moves backward, reverse that side's ports in the `MotorGroup` constructor.

## Arcade drive

Arcade drive uses one stick for forward motion and one stick for turning.

```cpp
void opcontrol() {
    pros::Controller controller(pros::E_CONTROLLER_MASTER);

    while (true) {
        const double throttle = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0;
        const double turn = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X) / 127.0;

        left_motors.move(throttle + turn);
        right_motors.move(throttle - turn);

        pros::delay(10);
    }
}
```

If the robot turns the wrong way, swap the signs on `turn`.

## Brake mode

Set a brake mode during initialization.

```cpp
void initialize() {
    pros::lcd::initialize();

    left_motors.setBrakeMode(ll::BrakeMode::BRAKE);
    right_motors.setBrakeMode(ll::BrakeMode::BRAKE);

    imu.calibrate();
    pros::delay(3000);
    odom.startTask();
}
```

Use `COAST` for smoother manual driving, `BRAKE` for predictable autonomous stops, and `HOLD` only when you need the drivetrain to resist movement while stopped.

## Add a deadband

A deadband prevents tiny joystick values from moving the robot.

```cpp
double apply_deadband(double value) {
    if (std::abs(value) < 0.05) return 0;
    return value;
}
```

Use it before commanding the motors.

```cpp
const double throttle = apply_deadband(
    controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0
);
```

## Display odometry while driving

Keep an eye on pose values while you drive by hand.

```cpp
const auto pose = odom.getPose();
pros::lcd::print(0, "X: %.1f", to_in(pose.x));
pros::lcd::print(1, "Y: %.1f", to_in(pose.y));
pros::lcd::print(2, "H: %.1f", to_cDeg(pose.orientation));
```

Drive forward 24 inches. If the position changes backward or sideways, fix tracking wheel direction and offsets before tuning autonomous.

## Pre-autonomous checks

- Both sides move forward with positive motor power.
- The robot can drive straight by hand.
- The heading changes when the robot turns.
- X and Y change in a way that matches your coordinate system.
- Brake mode feels predictable at the end of driver movements.

Next: [PID Tuning](./4_pid_tuning.md)
