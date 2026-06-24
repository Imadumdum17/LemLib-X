# 8 - Motion Chaining

Autonomous routines are built from short, dependable motions. LemLib-X motions run through the motion handler, so you can either wait for each motion or overlap robot actions while a motion is running.

## Waiting between motions

Start with fully blocking sequences. They are easiest to debug.

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

Get this version reliable before trying to save time.

## Run mechanisms during motion

Because motions are asynchronous, code after the motion call runs immediately.

```cpp
ll::moveToPoint({36_in, 0_in}, 5_sec, {}, {});

intake.move(1.0);
pros::delay(500);
intake.move(0);

waitUntilSettled();
```

This is useful for intakes, wings, lifts, and other mechanisms that can run while the drivetrain moves.

## Start the next motion early

Use `minLateralSpeed` and `earlyExitRange` when you want a motion to hand off before perfectly settling.

```cpp
ll::MoveToPointParams fast;
fast.minLateralSpeed = 0.15;
fast.earlyExitRange = 3_in;

ll::moveToPoint({36_in, 0_in}, 4_sec, fast, {});
waitUntilSettled();

ll::turnTo(90_cDeg, 2_sec, {}, {});
waitUntilSettled();
```

Only use early exits after the normal settled version works.

## Cancel when strategy changes

Cancel the current motion before switching to a different plan.

```cpp
if (goal_is_blocked()) {
    mh::cancel();
    pros::delay(20);

    ll::moveToPoint({12_in, -12_in}, 3_sec, {.reversed = true}, {});
}
```

A short delay gives the current motion task time to stop before the next motion starts.

## Build named routine sections

Small functions make autonomous easier to tune.

```cpp
void score_preload() {
    ll::moveToPose({30_in, 8_in, 45_cDeg}, 5_sec, {}, {});
    waitUntilSettled();

    outtake.move(1.0);
    pros::delay(400);
    outtake.move(0);
}

void collect_center() {
    intake.move(1.0);

    ll::follow(center_path_txt, 8_in, 7_sec, {}, {});
    waitUntilSettled();

    intake.move(0);
}

void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    score_preload();
    collect_center();
}
```

## Debugging chained routines

When a sequence fails, isolate the smallest section that reproduces it.

1. Run only the first motion.
2. Add the next mechanism action.
3. Add the next motion.
4. Retune timeouts and exits only after each piece works alone.

Most chaining problems come from an earlier motion ending in a different pose than the next motion expects.

## Final checklist

- Every routine sets the starting pose.
- Each motion has a realistic timeout.
- Mechanisms are stopped when they no longer need to run.
- Early exits are used only where they improve match time.
- The routine still works on a lower battery.
