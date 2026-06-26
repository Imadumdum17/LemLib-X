# 4 - PID Tuning

PID tuning is the process of making the robot approach a target quickly, smoothly, and repeatably. LemLib-X uses separate PID controllers for lateral movement and angular movement.

Tune one controller at a time. Change one value at a time. Keep notes.

## Tuning Setup

Start with conservative values:

```cpp
ll::PID lateral_pid(0.5, 0, 0);
ll::PID angular_pid(0.8, 0, 0.03);
```

Use consistent test conditions:

- fresh or known battery voltage
- same starting pose each run
- same field surface
- same robot weight
- wheels clean and not slipping excessively

Add a helper to wait for motions:

```cpp
void waitUntilSettled() {
    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

## Tune Lateral kP

Use a short straight motion.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});
    waitUntilSettled();
}
```

Increase lateral `kP` until the robot responds quickly but starts to overshoot or oscillate.

```cpp
ll::PID lateral_pid(0.7, 0, 0);
```

Guideline:

| Behavior | Change |
| --- | --- |
| Too slow | Increase `kP` |
| Stops far short | Increase `kP` or check friction |
| Overshoots a lot | Decrease `kP` |
| Oscillates repeatedly | Decrease `kP` |

## Add Lateral kD

Derivative gain dampens the final approach.

```cpp
ll::PID lateral_pid(0.7, 0, 0.04);
```

Increase `kD` until overshoot is controlled. If the robot becomes twitchy or noisy, reduce `kD`.

## Add Lateral kI Only If Needed

Integral gain helps with a consistent final error. It is not a cure for poor mechanics or bad `kP`.

```cpp
ll::PID lateral_pid(0.7, 0.01, 0.04);
```

Use tiny values. If `kI` makes the robot creep, surge, or oscillate near the target, remove it.

## Tune Angular Motion

Once lateral motion is reasonable, tune `turnTo`.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::turnTo(90_cDeg, 3_sec, {}, {});
    waitUntilSettled();
}
```

Tune angular values with the same process:

1. Increase angular `kP` until turns are fast but begin to overshoot.
2. Add angular `kD` until overshoot is controlled.
3. Add angular `kI` only if the robot consistently stops a little short.

## Test Multiple Distances

A tune that works at 24 inches may not work at 60 inches. Test several distances:

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({12_in, 0_in}, 3_sec, {}, {});
    waitUntilSettled();

    pros::delay(500);
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({48_in, 0_in}, 5_sec, {}, {});
    waitUntilSettled();
}
```

If short motions are good but long motions slip, reduce max speed or slew before changing PID.

## Tune Exit Conditions

Exit conditions decide when a motion ends.

```cpp
const ll::ExitConditionGroup<Length> lateral_exit_conditions({
    ll::ExitCondition<Length>(1_in, 200_msec),
    ll::ExitCondition<Length>(3_in, 600_msec),
});
```

If the robot is accurate but waits too long, reduce settle time. If it exits too early, tighten the range or increase settle time.

Do not use loose exit conditions to hide bad tuning. First make the approach smooth, then optimize exit timing.

## Tune Slew and Speed

If the robot slips at the beginning of a motion, PID is not the first thing to change. Use speed and slew limits.

```cpp
ll::MoveToPointParams params;
params.maxLateralSpeed = 0.6;
params.lateralSlew = 5;

ll::moveToPoint({48_in, 0_in}, 5_sec, params, {});
```

Lower max speed when:

- the robot tips
- wheels spin
- odometry jumps during acceleration
- the robot cannot repeat the same stop

## Keep a Tuning Log

Record what changed and what happened.

```cpp
// 2026-06-26
// 6 motor drive, 360 rpm output, 2.75 in omnis, full battery.
// 24 in: stops within 0.8 in. 48 in: slight overshoot.
ll::PID lateral_pid(0.72, 0, 0.045);
ll::PID angular_pid(1.05, 0, 0.075);
```

## Symptom Table

| Behavior | Likely cause | Try |
| --- | --- | --- |
| Slow response | `kP` too low | Increase `kP` |
| Big overshoot | `kP` too high or `kD` too low | Lower `kP`, raise `kD` |
| Fast jitter | `kD` too high or sensor noise | Lower `kD`, inspect sensors |
| Ends short every time | Not enough output or small steady error | Increase `kP`, then tiny `kI` |
| Works once but not repeatedly | Slip, battery, or bad odometry | Lower speed, verify tracking |
| Turns drift in one direction | IMU or drivetrain asymmetry | Recalibrate, inspect friction |

Next: [Angular Motion](./5_angular_motion.md)
