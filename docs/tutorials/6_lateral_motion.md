# 6 - Lateral Motion

Lateral motion moves the robot across the field. Use `moveToPoint` for simple travel to a point, and `moveToPose` when the final heading matters.

This tutorial covers:

- choosing between `moveToPoint` and `moveToPose`
- forward and backward moves
- speed limits
- minimum speed and early exits
- lead tuning
- drift compensation
- practical autonomous patterns

## Choose the Right Motion

| Motion | Use when |
| --- | --- |
| `moveToPoint` | You only care about the target X/Y position |
| `moveToPose` | You care about X/Y and final heading |
| `turnTo` | You only need to rotate |
| `follow` | You need to follow a curved path |

Start with `moveToPoint`. It is easier to tune because it has fewer goals.

## Drive to a Point

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({36_in, 0_in}, 5_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

The timeout is a safety limit, not the expected time. Give early tests generous timeouts.

## Drive Backward

Set `reversed` when the robot should drive backward toward the target.

```cpp
ll::MoveToPointParams params;
params.reversed = true;

ll::moveToPoint({-24_in, 0_in}, 4_sec, params, {});
```

Backward motion is useful when the scoring side or intake side must keep facing a specific direction.

## Control Speed

Limit speed when the robot slips, carries game objects, or approaches a wall.

```cpp
ll::MoveToPointParams params;
params.maxLateralSpeed = 0.55;
params.maxAngularSpeed = 0.45;

ll::moveToPoint({48_in, 0_in}, 6_sec, params, {});
```

Lower max speed before changing PID if the robot cannot repeat the motion.

## Minimum Speed and Early Exit

Minimum speed can help a heavy robot keep moving, but it can also force overshoot.

```cpp
ll::MoveToPointParams params;
params.minLateralSpeed = 0.12;
params.earlyExitRange = 2_in;

ll::moveToPoint({36_in, 0_in}, 4_sec, params, {});
```

When using minimum speed, give the motion a reasonable early exit range so it can hand off instead of fighting to settle perfectly.

## Move to a Pose

`moveToPose` drives to a target point while also controlling final heading.

```cpp
ll::moveToPose({36_in, 24_in, 90_cDeg}, 6_sec, {}, {});
```

Use this for:

- scoring positions
- match load positions
- ending next to a field object
- preparing for a path or turn

## Tune Lead

`lead` controls how far ahead of the target the robot aims while approaching.

```cpp
ll::MoveToPoseParams params;
params.lead = 0.6;

ll::moveToPose({36_in, 24_in, 90_cDeg}, 6_sec, params, {});
```

General behavior:

| Lead | Behavior |
| --- | --- |
| Lower | More direct, sharper approach |
| Higher | Smoother approach, needs more room |

If the robot hooks sharply into the target, raise lead slightly. If it cuts too wide, lower lead.

## Prevent Slipping

For curved approaches, use speed limits and drift compensation.

```cpp
ll::MoveToPoseParams params;
params.maxLateralSpeed = 0.55;
params.maxAngularSpeed = 0.55;
params.driftCompensation = 0.5;

ll::moveToPose({48_in, 24_in, 90_cDeg}, 7_sec, params, {});
```

Retune after changing:

- wheel type
- drivetrain gearing
- robot weight
- center of gravity
- field surface

## Helper Pattern

Wrap waiting in one helper to keep autonomous readable.

```cpp
void waitUntilSettled() {
    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

Then build simple sequences:

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});
    waitUntilSettled();

    ll::moveToPose({48_in, 24_in, 90_cDeg}, 6_sec, {}, {});
    waitUntilSettled();
}
```

## Motion Debugging

Print pose before and after each motion.

```cpp
void printPoseLine(const char* label) {
    const auto pose = odom.getPose();
    printf("%s: %.1f %.1f %.1f\n",
           label,
           to_in(pose.x),
           to_in(pose.y),
           to_cDeg(pose.orientation));
}
```

Use it like this:

```cpp
printPoseLine("before");
ll::moveToPoint({24_in, 0_in}, 4_sec, {}, {});
waitUntilSettled();
printPoseLine("after");
```

## Troubleshooting Lateral Motion

| Problem | Likely cause | What to try |
| --- | --- | --- |
| Robot drives wrong way | Pose or reversed flag wrong | Check starting pose and `params.reversed` |
| Robot curves badly | Angular PID or motor direction | Tune angular PID, check drivetrain |
| Stops short | Low `kP`, friction, or strict exit | Raise `kP`, inspect mechanics |
| Overshoots | Too much speed or `kP` | Lower speed, add `kD` |
| Position jumps | Bad odometry | Check tracking wheel direction and offsets |

Next: [Pure Pursuit](./7_pure_pursuit.md)
