# 6 - Lateral Motion

Use `moveToPoint` for simple driving to a point and `moveToPose` when the final heading matters.

## Drive to a point

`moveToPoint` moves the robot toward an X/Y target.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({36_in, 0_in}, 5_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

This is the best first lateral motion because it has fewer moving parts than `moveToPose`.

## Drive backward

Set `reversed` when the robot should drive backward toward the target.

```cpp
ll::MoveToPointParams params;
params.reversed = true;

ll::moveToPoint({-24_in, 0_in}, 4_sec, params, {});
```

Backward moves are often useful when the intake or scoring side of the robot should keep facing forward.

## Control speed

Use max speed to make a motion gentler.

```cpp
ll::MoveToPointParams params;
params.maxLateralSpeed = 0.55;
params.maxAngularSpeed = 0.45;

ll::moveToPoint({48_in, 0_in}, 6_sec, params, {});
```

Use minimum speed carefully. It helps a robot push through friction, but it can also make the robot overshoot.

```cpp
params.minLateralSpeed = 0.12;
params.earlyExitRange = 2_in;
```

When `minLateralSpeed` is nonzero, `earlyExitRange` lets the motion finish once the robot crosses near the target.

## Move to a pose

`moveToPose` drives to a point while also controlling final heading.

```cpp
ll::moveToPose({36_in, 24_in, 90_cDeg}, 6_sec, {}, {});
```

Use this for scoring positions, match load positions, and any target where orientation matters.

## Tune the lead value

`lead` controls how far ahead of the target the robot aims while approaching.

```cpp
ll::MoveToPoseParams params;
params.lead = 0.6;

ll::moveToPose({36_in, 24_in, 90_cDeg}, 6_sec, params, {});
```

Lower lead makes the robot aim more directly at the target. Higher lead makes the approach smoother but can need more space.

## Prevent slipping

If the robot slides during curved approaches, lower speed first.

```cpp
ll::MoveToPoseParams params;
params.maxLateralSpeed = 0.55;
params.maxAngularSpeed = 0.55;
params.driftCompensation = 0.5;

ll::moveToPose({48_in, 24_in, 90_cDeg}, 7_sec, params, {});
```

Retune after changing wheel type, weight distribution, or drive gearing.

## Common pattern

Wrap waiting in a helper to keep autonomous readable.

```cpp
void waitUntilSettled() {
    while (mh::isMoving()) {
        pros::delay(10);
    }
}

void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});
    waitUntilSettled();

    ll::moveToPose({48_in, 24_in, 90_cDeg}, 6_sec, {}, {});
    waitUntilSettled();
}
```

Next: [Pure Pursuit](./7_pure_pursuit.md)
