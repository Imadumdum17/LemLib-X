# 7 - Pure Pursuit

Pure Pursuit follows a path made from points instead of driving directly to one target. In LemLib-X, the `follow` motion reads a path from a PROS asset.

Use Pure Pursuit when:

- the robot needs a smooth curved path
- a direct line would hit an obstacle
- you want to control speed through different parts of a route
- a long autonomous route is easier to describe as waypoints

## Path File Format

Put path files in the project `static` folder. Each line contains:

```text
x, y, speed
```

Use inches for `x` and `y`. Use a speed from `0` to `1`. The final point should have speed `0`.

```text
0, 0, 0.4
12, 8, 0.6
30, 18, 0.7
48, 24, 0
endData
```

Save this as `static/skills_path.txt`.

The `endData` line is required. Without it, the parser may read invalid data.

## Load the Asset

Declare the asset at file scope, outside any function.

```cpp
#include "hot-cold-asset/asset.hpp"

ASSET(skills_path_txt);
```

PROS converts the filename into the symbol name. Dots become underscores, so `skills_path.txt` becomes `skills_path_txt`.

## Follow the Path

Call `follow` with the asset, a lookahead distance, and a timeout.

```cpp
void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::follow(skills_path_txt, 8_in, 8_sec, {}, {});

    while (mh::isMoving()) {
        pros::delay(10);
    }
}
```

Start with a lookahead between `6_in` and `10_in`.

## Understand Lookahead

Lookahead is how far ahead on the path the robot aims.

| Lookahead | Behavior |
| --- | --- |
| Smaller | More precise, can oscillate |
| Larger | Smoother, may cut corners |

If the robot weaves across the path, increase lookahead slightly or reduce path speed. If it cuts corners too much, reduce lookahead or add more points around the curve.

## Tune Path Speed

Path speed is a normalized motor command. Use lower speeds in tight turns.

```text
0, 0, 0.35
18, 8, 0.45
36, 18, 0.55
48, 24, 0
endData
```

Start slow. A path that works slowly can be sped up. A path that fails quickly is hard to diagnose.

## Add More Points

For smooth curves, add intermediate points.

```text
0, 0, 0.35
8, 2, 0.45
16, 7, 0.55
24, 14, 0.55
36, 22, 0.5
48, 24, 0
endData
```

Avoid large sudden direction changes between points. Pure Pursuit follows geometry; it does not magically make an impossible path possible.

## Follow in Reverse

Set `reversed` when the robot should drive backward along the path.

```cpp
ll::FollowParams params;
params.reversed = true;

ll::follow(skills_path_txt, 8_in, 8_sec, params, {});
```

Make sure the starting pose matches the robot's real location and heading before starting the path.

## Use Slew

Slew smooths speed changes along the path.

```cpp
ll::FollowParams params;
params.lateralSlew = 5;

ll::follow(skills_path_txt, 8_in, 8_sec, params, {});
```

If the robot slips at the beginning of the path, lower the first path speed and reduce acceleration with slew.

## Path Debugging Routine

Use one path per test routine.

```cpp
void waitUntilSettled() {
    while (mh::isMoving()) {
        pros::delay(10);
    }
}

void autonomous() {
    odom.setPose({0_in, 0_in, 0_cDeg});

    ll::follow(skills_path_txt, 8_in, 10_sec, {}, {});
    waitUntilSettled();
}
```

Print the final pose after the path. If the robot ends far from expected, check odometry before changing path points.

## Path Checklist

- The file is inside `static`.
- The asset name matches the file name with dots changed to underscores.
- Each path line has exactly `x, y, speed`.
- The final point has speed `0`.
- The file ends with `endData`.
- The robot starts near the first point.
- Lookahead is not so small that the robot weaves.
- Speeds are low enough for the robot to stay on the path.

## Troubleshooting Pure Pursuit

| Problem | Likely cause | What to try |
| --- | --- | --- |
| Path does not load | Asset name or file format wrong | Check `ASSET(...)`, commas, and `endData` |
| Robot turns away immediately | Starting pose wrong | Reset pose to match first path point |
| Robot weaves | Lookahead too small or speed too high | Increase lookahead, lower speed |
| Robot cuts corners | Lookahead too large or too few points | Lower lookahead, add points |
| Robot falls behind path | Path too fast | Lower speed values |

Next: [Motion Chaining](./8_motion_chaining.md)
