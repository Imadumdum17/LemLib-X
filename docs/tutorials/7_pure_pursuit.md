# 7 - Pure Pursuit

Pure Pursuit follows a path made of points instead of driving directly to one target. In LemLib-X, the `follow` motion reads a path from a PROS asset.

## Create a path file

Put path files in the project `static` folder. Each line contains:

```text
x, y, speed
```

Use inches for `x` and `y`. Use `0` speed at the final point so `follow` knows the path is complete.

```text
0, 0, 0.4
12, 8, 0.6
30, 18, 0.7
48, 24, 0
endData
```

For example, save this as `static/skills_path.txt`.

## Load the asset

Declare the asset at file scope, outside any function.

```cpp
#include "hot-cold-asset/asset.hpp"

ASSET(skills_path_txt);
```

PROS converts the filename into the symbol name. Dots become underscores, so `skills_path.txt` becomes `skills_path_txt`.

## Follow the path

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

## Tune lookahead

| Lookahead | Behavior |
| --- | --- |
| Smaller | More precise, can oscillate |
| Larger | Smoother, cuts corners more |

If the robot weaves across the path, increase lookahead slightly or reduce path speed. If the robot cuts corners too much, reduce lookahead or add more path points.

## Tune path speed

Path speed is a normalized motor command, usually from `0` to `1`.

```text
0, 0, 0.35
18, 8, 0.45
36, 18, 0.55
48, 24, 0
endData
```

Use lower speed through tight turns and higher speed on long straight sections.

## Follow in reverse

Set `reversed` when the robot should drive backward along the path.

```cpp
ll::FollowParams params;
params.reversed = true;

ll::follow(skills_path_txt, 8_in, 8_sec, params, {});
```

Make sure the starting pose matches the robot's real location and heading before starting the path.

## Path checklist

- The path starts near the robot's starting pose.
- The final point has speed `0`.
- The file ends with `endData`.
- Speeds are low enough for the robot to stay on the path.
- Lookahead is large enough to avoid weaving.

Next: [Motion Chaining](./8_motion_chaining.md)
