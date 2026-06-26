#include "hardware/Motor/Motor.hpp"

#include "pros/error.h"
#include <algorithm>
#include <cmath>
#include <climits>
#include <cstdint>

namespace {
pros::motor_brake_mode_e_t toProsBrakeMode(lemlib_x::BrakeMode mode) {
    switch (mode) {
        case lemlib_x::BrakeMode::BRAKE: return pros::E_MOTOR_BRAKE_BRAKE;
        case lemlib_x::BrakeMode::HOLD: return pros::E_MOTOR_BRAKE_HOLD;
        case lemlib_x::BrakeMode::COAST: return pros::E_MOTOR_BRAKE_COAST;
        default: return pros::E_MOTOR_BRAKE_INVALID;
    }
}

lemlib_x::BrakeMode fromProsBrakeMode(pros::motor_brake_mode_e_t mode) {
    switch (mode) {
        case pros::E_MOTOR_BRAKE_BRAKE: return lemlib_x::BrakeMode::BRAKE;
        case pros::E_MOTOR_BRAKE_HOLD: return lemlib_x::BrakeMode::HOLD;
        case pros::E_MOTOR_BRAKE_COAST: return lemlib_x::BrakeMode::COAST;
        default: return lemlib_x::BrakeMode::INVALID;
    }
}
} // namespace

namespace lemlib_x {
Motor::Motor(ReversibleSmartPort port, AngularVelocity outputVelocity)
    : m_outputVelocity(outputVelocity),
      m_port(port) {}

Motor Motor::from_pros_motor(const pros::Motor motor, AngularVelocity outputVelocity) {
    return Motor(ReversibleSmartPort(motor.get_port(), runtime_check_port), outputVelocity);
}

int Motor::move(Number percent) {
    const int voltage = std::clamp(static_cast<int>(static_cast<double>(percent) * 127.0), -127, 127);
    return static_cast<int>(pros::Motor(static_cast<std::int8_t>(m_port)).move(voltage));
}

int Motor::moveVelocity(AngularVelocity velocity) {
    return static_cast<int>(pros::Motor(static_cast<std::int8_t>(m_port)).move_velocity(static_cast<int>(to_rpm(velocity))));
}

int Motor::brake() { return static_cast<int>(pros::Motor(static_cast<std::int8_t>(m_port)).brake()); }

int Motor::setBrakeMode(BrakeMode mode) {
    return static_cast<int>(pros::Motor(static_cast<std::int8_t>(m_port)).set_brake_mode(toProsBrakeMode(mode)));
}

BrakeMode Motor::getBrakeMode() const {
    return fromProsBrakeMode(pros::Motor(static_cast<std::int8_t>(m_port)).get_brake_mode());
}

int Motor::isConnected() { return pros::Motor(static_cast<std::int8_t>(m_port)).is_installed() ? 1 : 0; }

Angle Motor::getAngle() {
    const double position = pros::Motor(static_cast<std::int8_t>(m_port)).get_position();
    if (!std::isfinite(position)) return Angle(INFINITY);
    return position * deg + m_offset;
}

int Motor::setAngle(Angle angle) {
    const double position = pros::Motor(static_cast<std::int8_t>(m_port)).get_position();
    if (!std::isfinite(position)) return INT_MAX;
    m_offset = angle - position * deg;
    return 0;
}

Angle Motor::getOffset() const { return m_offset; }

int Motor::setOffset(Angle offset) {
    m_offset = offset;
    return 0;
}

MotorType Motor::getType() {
    if (!pros::Motor(static_cast<std::int8_t>(m_port)).is_installed()) return MotorType::INVALID;
    return MotorType::V5;
}

int Motor::isReversed() const { return m_port.is_reversed() ? 1 : 0; }

int Motor::setReversed(bool reversed) {
    m_port = m_port.set_reversed(reversed);
    return static_cast<int>(pros::Motor(static_cast<std::int8_t>(m_port)).set_reversed(reversed));
}

ReversibleSmartPort Motor::getPort() const { return m_port; }

Current Motor::getCurrentLimit() const {
    const std::int32_t limit = pros::Motor(static_cast<std::int8_t>(m_port)).get_current_limit();
    if (limit == PROS_ERR) return Current(INFINITY);
    return static_cast<double>(limit) / 1000.0 * amp;
}

int Motor::setCurrentLimit(Current limit) {
    return static_cast<int>(pros::Motor(static_cast<std::int8_t>(m_port)).set_current_limit(static_cast<std::int32_t>(to_amp(limit) * 1000)));
}

Temperature Motor::getTemperature() const {
    const double temp = pros::Motor(static_cast<std::int8_t>(m_port)).get_temperature();
    if (!std::isfinite(temp)) return Temperature(INFINITY);
    return units::from_celsius(temp);
}

int Motor::setOutputVelocity(AngularVelocity outputVelocity) {
    m_outputVelocity = outputVelocity;
    return 0;
}
} // namespace lemlib_x
