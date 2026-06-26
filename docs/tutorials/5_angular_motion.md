# 5 - Angular Motion

Angular motion controls where the robot faces. Use `turnTo` when the next action depends on orientation: lining up with a goal, facing an object, or preparing for a path.

This tutorial covers:

- turning to a heading
- turning to face a point
- speed limits
- slew
- forced direction
- swing turns
- cancellation
- troubleshooting turns

## Turn to a Heading

The simplest turn targets a field-relative heading.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::turnTo(90_cDeg, 3_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

The target is not "turn 90 degrees from now." It is "face the field heading 90 degrees." If the robot starts at `45_cDeg` and you call `turnTo(90_cDeg, ...)`, it turns about 45 degrees.

## Turn to Face a Point

`turnTo` can also accept a position. The robot calculates the heading needed to face that point.

```cpp
ll::turnTo(units::V2Position{24_in, 24_in}, 3_sec, {}, {});
```

Use this when the target is an object or location on the field and you do not want to calculate the heading yourself.

## Limit Turn Speed

Lower max speed when precision matters more than time.

```cpp
ll::TurnToParams slow_turn;
slow_turn.maxSpeed = 0.45;

ll::turnTo(135_cDeg, 4_sec, slow_turn, {});
```

`maxSpeed` is normalized from `0` to `1`. A value around `0.4` to `0.6` is useful for early tuning.

## Use Minimum Speed Carefully

Minimum speed helps overcome static friction, but it can make turns overshoot.

```cpp
ll::TurnToParams params;
params.minSpeed = 0.08;
params.earlyExitRange = 3_stDeg;

ll::turnTo(90_cDeg, 3_sec, params, {});
```

When `minSpeed` is nonzero, use `earlyExitRange` so the motion can finish instead of forcing power through the target.

## Use Slew for Smoother Starts

Slew limits how quickly turn power can rise.

```cpp
ll::TurnToParams smooth_turn;
smooth_turn.slew = 5;

ll::turnTo(180_cDeg, 4_sec, smooth_turn, {});
```

If the robot snaps into turns and loses traction, lower `maxSpeed` or use a gentler slew.

## Force a Direction

By default, LemLib-X chooses the shorter turn. You can force a direction for strategy or mechanism clearance.

```cpp
ll::TurnToParams params;
params.direction = ll::AngularDirection::CW_CLOCKWISE;

ll::turnTo(270_cDeg, 4_sec, params, {});
```

Only force direction when you need it. Automatic direction is usually faster and simpler.

## Swing Turns

Swing turns lock one side of the drivetrain and rotate around it.

```cpp
ll::TurnToParams swing;
swing.lockedSide = ll::TurnToParams::LockedSide::LEFT;
swing.maxSpeed = 0.5;

ll::turnTo(45_cDeg, 4_sec, swing, {});
```

Swing turns are useful near walls or game objects, but they need more room on the moving side. Tune them separately from normal turns.

## Cancel a Turn

Motions run through the motion handler and can be cancelled.

```cpp
ll::turnTo(180_cDeg, 5_sec, {}, {});

if (should_stop_turning()) {
    mh::cancel();
    pros::delay(20);
}
```

Wait briefly after cancelling before starting another motion so the task has time to stop.

## Build a Turn Test Routine

Test several headings in one routine.

```cpp
void waitUntilSettled() {
    while (mh::isMoving()) {
        pros::delay(10);
    }
}

void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::turnTo(90_cDeg, 3_sec, {}, {});
    waitUntilSettled();
    pros::delay(500);

    ll::turnTo(180_cDeg, 3_sec, {}, {});
    waitUntilSettled();
    pros::delay(500);

    ll::turnTo(0_cDeg, 3_sec, {}, {});
    waitUntilSettled();
}
```

Watch whether the robot is equally good clockwise and counterclockwise. If one direction is much worse, inspect drivetrain friction and motor reversal.

## Troubleshooting Turns

| Problem | Likely cause | What to try |
| --- | --- | --- |
| Overshoots | `kP` too high or `kD` too low | Lower angular `kP`, raise `kD` |
| Oscillates | Too aggressive | Lower `kP`, lower max speed |
| Starts too harshly | Too much acceleration | Add slew or lower max speed |
| Stops short | `kP` too low or friction | Raise `kP`, add tiny `minSpeed` |
| Wrong final heading | Starting pose or IMU wrong | Reset pose, recalibrate IMU |
| Turns wrong direction | Coordinate assumption wrong | Verify heading sign on screen |

Next: [Lateral Motion](./6_lateral_motion.md)
