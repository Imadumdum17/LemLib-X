#pragma once

#include "lemlib/config.hpp"
#include "hot-cold-asset/asset.hpp"

namespace lemlib_x {
struct FollowParams {
        bool reversed = false;
        Number lateralSlew = lateral_slew;
};

struct FollowSettings {
        Length trackWidth = track_width;
        std::function<units::Pose()> poseGetter = pose_getter;
        MotorGroup& leftMotors = left_motors;
        MotorGroup& rightMotors = right_motors;
};

void follow(const asset& path, Length lookaheadDistance, Time timeout, FollowParams params, FollowSettings settings);
} // namespace lemlib_x