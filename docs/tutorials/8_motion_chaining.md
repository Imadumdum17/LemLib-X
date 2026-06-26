# 8 - Motion Chaining

Autonomous routines are built from short, dependable motions. LemLib-X motions run through the motion handler, so code after a motion call runs immediately.

This tutorial shows how to build routines that are readable, testable, and fast enough for competition.

## Start with Blocking Motion

The safest pattern is to wait after each motion.

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

    ll::turnTo(90_cDeg, 3_sec, {}, {});
    waitUntilSettled();

    ll::moveToPose({24_in, 24_in, 90_cDeg}, 5_sec, {}, {});
    waitUntilSettled();
}
```

Get the blocking version reliable before optimizing time.

## Run Mechanisms During Motion

Because motions are asynchronous, mechanisms can run while the drivetrain moves.

```cpp
ll::moveToPoint({36_in, 0_in}, 5_sec, {}, {});

intake.move(1.0);
pros::delay(500);
intake.move(0);

waitUntilSettled();
```

This is useful for intakes, lifts, wings, clamps, and other mechanisms that can safely move while driving.

## Wait for Timing, Then Wait for Motion

Sometimes a mechanism needs to run for a fixed time during a drive.

```cpp
ll::moveToPoint({30_in, 0_in}, 5_sec, {}, {});

intake.move(1.0);
pros::delay(700);
intake.move(0);

waitUntilSettled();
```

If the drive finishes before the mechanism timing ends, this code still waits for the mechanism. If the mechanism finishes first, it waits for the drive.

## Start the Next Motion Early

Use `minLateralSpeed` and `earlyExitRange` when a motion should hand off before perfectly settling.

```cpp
ll::MoveToPointParams fast;
fast.minLateralSpeed = 0.15;
fast.earlyExitRange = 3_in;

ll::moveToPoint({36_in, 0_in}, 4_sec, fast, {});
waitUntilSettled();

ll::turnTo(90_cDeg, 2_sec, {}, {});
waitUntilSettled();
```

Only use early exits after the normal settled version works. Early exits make routines faster, but they also reduce margin for error.

## Cancel When Strategy Changes

Cancel the current motion before switching to a different plan.

```cpp
if (goal_is_blocked()) {
    mh::cancel();
    pros::delay(20);

    ll::moveToPoint({12_in, -12_in}, 3_sec, {.reversed = true}, {});
}
```

A short delay gives the current motion task time to stop before the next motion starts.

## Build Named Routine Sections

Small functions make autonomous easier to tune.

```cpp
void scorePreload() {
    ll::moveToPose({30_in, 8_in, 45_cDeg}, 5_sec, {}, {});
    waitUntilSettled();

    outtake.move(1.0);
    pros::delay(400);
    outtake.move(0);
}

void collectCenter() {
    intake.move(1.0);

    ll::follow(center_path_txt, 8_in, 7_sec, {}, {});
    waitUntilSettled();

    intake.move(0);
}

void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    scorePreload();
    collectCenter();
}
```

Prefer names that describe strategy, not implementation. `scorePreload()` is easier to understand than `move1()`.

## Add Checkpoints

Print pose between sections while tuning.

```cpp
void logPose(const char* label) {
    const auto pose = odom.getPose();
    printf("%s: %.1f %.1f %.1f\n",
           label,
           to_in(pose.x),
           to_in(pose.y),
           to_cDeg(pose.orientation));
}
```

Use checkpoints:

```cpp
scorePreload();
logPose("after preload");

collectCenter();
logPose("after center");
```

When a long routine fails, checkpoints show which section introduced the error.

## Tune One Section at a Time

When a sequence fails, isolate the smallest section that reproduces it.

1. Run only the first motion.
2. Add the next mechanism action.
3. Add the next motion.
4. Add timing overlap.
5. Add early exits last.

Most chaining problems come from an earlier motion ending in a different pose than the next motion expects.

## Battery and Field Validation

Test the routine under match-like conditions:

- fresh battery
- lower battery
- game objects loaded
- robot starting slightly off
- field surface similar to competition

If a routine only works with a perfect setup, slow it down or add more forgiving exits.

## Final Checklist

- Every autonomous routine sets the starting pose.
- Each motion has a realistic timeout.
- Mechanisms stop when they no longer need to run.
- Early exits are used only where they improve match time.
- Pose is logged between major sections while tuning.
- The routine works more than once in a row.
- The routine still works on a lower battery.
