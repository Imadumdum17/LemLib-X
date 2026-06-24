#pragma once

// this file is used to configure default values used by motion algorithms used in LemLib

#include "ExitCondition.hpp"
#include "PID.hpp"
#include "hardware/Motor/MotorGroup.hpp"
#include "units/Pose.hpp"
#include <functional>

extern lemlib_x::PID angular_pid;
extern lemlib_x::PID lateral_pid;

extern const std::function<units::Pose()> pose_getter;

extern lemlib_x::MotorGroup left_motors;
extern lemlib_x::MotorGroup right_motors;

extern const lemlib_x::ExitConditionGroup<AngleRange> angular_exit_conditions;
extern const lemlib_x::ExitConditionGroup<Length> lateral_exit_conditions;

extern const Length track_width;

extern const Number drift_compensation;

extern const Number angular_slew;
extern const Number lateral_slew;

// functions to set PID values at runtime
void setAngularPID(const lemlib_x::PID& pid);
void setLateralPID(const lemlib_x::PID& pid);