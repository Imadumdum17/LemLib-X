# 4 - PID Tuning

Tune one controller at a time. Start with slow, short motions and make one change between tests.

## Start with lateral motion

Set angular gains low enough that the robot points toward the target without fighting the lateral tune.

```cpp
ll::PID lateral_pid(0.5, 0, 0);
ll::PID angular_pid(0.8, 0, 0.03);
```

Use a short autonomous test.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

## Tune kP

Increase `kP` until the robot reaches the target quickly but starts to overshoot or oscillate.

```cpp
ll::PID lateral_pid(0.7, 0, 0);
```

If the robot is slow and stops short, increase `kP`. If it overshoots hard, decrease `kP`.

## Add kD

Add derivative gain to damp the final approach.

```cpp
ll::PID lateral_pid(0.7, 0, 0.04);
```

If the robot jitters or sounds harsh, `kD` is probably too high.

## Use kI sparingly

Only add integral when the robot is stable but consistently ends with a small error.

```cpp
ll::PID lateral_pid(0.7, 0.01, 0.04);
```

If adding `kI` makes the robot creep, oscillate, or surge near the target, remove it and solve the issue with `kP`, `kD`, exit conditions, or mechanical checks.

## Tune angular motion

Once lateral motion is reasonable, tune turns with `turnTo`.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::turnTo(90_cDeg, 3_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

Use the same process:

1. Increase angular `kP` until the turn is fast but starts to overshoot.
2. Add angular `kD` until overshoot is controlled.
3. Add angular `kI` only if there is a consistent final heading error.

## Adjust exit conditions

If the robot is accurate but waits too long, reduce the settle time. If it ends too early, tighten the range or increase the settle time.

```cpp
const ll::ExitConditionGroup<Length> lateral_exit_conditions({
    ll::ExitCondition<Length>(1_in, 200_msec),
    ll::ExitCondition<Length>(3_in, 600_msec),
});
```

Do not use exit conditions to hide bad tuning. First make the robot approach the target smoothly, then adjust exits for match timing.

## Keep notes

Record each successful value with the robot setup and battery condition.

```cpp
// Tested 2026-06-24, 6-motor 360 rpm drive, fresh battery.
ll::PID lateral_pid(0.72, 0, 0.045);
ll::PID angular_pid(1.05, 0, 0.075);
```

## Symptoms

| Behavior | Likely change |
| --- | --- |
| Slow approach | Increase `kP` |
| Large overshoot | Decrease `kP` or increase `kD` |
| Fast shaking near target | Decrease `kD` |
| Repeated oscillation | Decrease `kP` |
| Stable but always short | Add tiny `kI` or check friction |

Next: [Angular Motion](./5_angular_motion.md)
