# 3 - Driver Control

Good autonomous starts with predictable driver control. Before tuning motions, make sure motors, brake modes, joystick scaling, and odometry feedback are all behaving.

This tutorial covers:

- tank drive
- arcade drive
- deadbands
- brake modes
- slow mode
- sensor display
- pre-autonomous validation

## Tank Drive

Tank drive maps one joystick to each side of the drivetrain.

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

Tank drive is the easiest mode for checking motor direction because each side is independent.

## Arcade Drive

Arcade drive uses one stick for forward movement and one stick for turning.

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

If the robot turns the wrong direction, swap the signs on `turn`.

## Add a Deadband

Controller sticks often report tiny values even when centered. A deadband prevents drift.

```cpp
double applyDeadband(double value) {
    if (std::abs(value) < 0.05) return 0;
    return value;
}
```

Use it on joystick inputs:

```cpp
const double throttle = applyDeadband(
    controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0
);
```

Keep the deadband small. A large deadband makes the robot feel jumpy when the stick leaves center.

## Add Slow Mode

Slow mode helps drivers line up carefully.

```cpp
double scaleDrive(double value, bool slow_mode) {
    return slow_mode ? value * 0.45 : value;
}
```

Use a controller button to enable it:

```cpp
const bool slow = controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1);

left_motors.move(scaleDrive(throttle + turn, slow));
right_motors.move(scaleDrive(throttle - turn, slow));
```

## Brake Modes

Set brake modes once during initialization.

```cpp
void initialize() {
    pros::lcd::initialize();

    left_motors.setBrakeMode(ll::BrakeMode::BRAKE);
    right_motors.setBrakeMode(ll::BrakeMode::BRAKE);

    imu.calibrate();
    while (imu.isCalibrating()) {
        pros::delay(20);
    }

    odom.startTask();
}
```

Use this guide:

| Mode | Behavior | Use for |
| --- | --- | --- |
| `COAST` | Rolls after power is removed | Smooth driver practice |
| `BRAKE` | Actively slows when stopped | Most autonomous |
| `HOLD` | Holds position | Rare drivetrain cases |

Avoid `HOLD` on the drivetrain unless you really need it; it can fight drivers and heat motors.

## Display Odometry While Driving

Print pose values so drivers can see whether odometry is plausible.

```cpp
void printPose() {
    const auto pose = odom.getPose();
    pros::lcd::print(0, "X: %.1f", to_in(pose.x));
    pros::lcd::print(1, "Y: %.1f", to_in(pose.y));
    pros::lcd::print(2, "H: %.1f", to_cDeg(pose.orientation));
}
```

Call it in the driver loop:

```cpp
while (true) {
    // drive code
    printPose();
    pros::delay(10);
}
```

## Full Driver Control Example

```cpp
double applyDeadband(double value) {
    if (std::abs(value) < 0.05) return 0;
    return value;
}

void opcontrol() {
    pros::Controller controller(pros::E_CONTROLLER_MASTER);

    while (true) {
        double throttle = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0;
        double turn = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X) / 127.0;

        throttle = applyDeadband(throttle);
        turn = applyDeadband(turn);

        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
            throttle *= 0.45;
            turn *= 0.45;
        }

        left_motors.move(throttle + turn);
        right_motors.move(throttle - turn);

        printPose();
        pros::delay(10);
    }
}
```

## Pre-Autonomous Validation

Before tuning autonomous, run these tests:

| Test | Expected result |
| --- | --- |
| Drive straight forward | Robot drives forward, not angled sharply |
| Rotate in place | Heading changes smoothly |
| Push forward by hand | X changes, Y stays mostly stable |
| Rotate by hand | Heading changes, position does not jump wildly |
| Stop from full speed | Brake mode feels predictable |

If the robot cannot drive straight manually, do not tune PID yet. Fix motors, gearing, wiring, friction, and sensor direction first.

Next: [PID Tuning](./4_pid_tuning.md)
