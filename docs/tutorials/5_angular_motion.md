# 5 - Angular Motion

Use `turnTo` when the robot needs to face a heading or point before the next action.

## Turn to a heading

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

Positive headings use the same angle units as the rest of LemLib-X. Be consistent with your team's coordinate convention when setting the starting pose.

## Turn to face a point

`turnTo` can also accept a position. The robot calculates the heading needed to face that point.

```cpp
ll::turnTo(units::V2Position{24_in, 24_in}, 3_sec, {}, {});
```

This is useful before intaking, scoring, or lining up with a field object.

## Limit turn speed

Lower the maximum speed when precision matters more than time.

```cpp
ll::TurnToParams slow_turn;
slow_turn.maxSpeed = 0.45;

ll::turnTo(135_cDeg, 4_sec, slow_turn, {});
```

Use values from `0` to `1`. A lower maximum speed can reduce overshoot on light or high-traction drivetrains.

## Use slew for smoother starts

Slew limits how quickly turn power can rise.

```cpp
ll::TurnToParams smooth_turn;
smooth_turn.slew = 5;

ll::turnTo(180_cDeg, 4_sec, smooth_turn, {});
```

If the robot snaps into turns and loses traction, reduce max speed or increase smoothing with slew.

## Force a direction

By default, LemLib-X chooses the shorter turn. You can force a direction for cable routing, mechanism clearance, or field strategy.

```cpp
ll::TurnToParams params;
params.direction = ll::AngularDirection::CW_CLOCKWISE;

ll::turnTo(270_cDeg, 4_sec, params, {});
```

Only force direction when you need it. The automatic direction is usually faster.

## Swing turns

Lock one side of the drivetrain to swing around it.

```cpp
ll::TurnToParams swing;
swing.lockedSide = ll::TurnToParams::LockedSide::LEFT;
swing.maxSpeed = 0.5;

ll::turnTo(45_cDeg, 4_sec, swing, {});
```

Swing turns are helpful near walls or game objects, but they need more room on the moving side of the robot.

## Cancel a turn

Motions run through the motion handler, so they can be cancelled.

```cpp
ll::turnTo(180_cDeg, 5_sec, {}, {});

if (should_stop_turning()) {
    mh::cancel();
}
```

After cancelling, give the motion task a short delay before starting another motion.

```cpp
mh::cancel();
pros::delay(20);
```

Next: [Lateral Motion](./6_lateral_motion.md)
