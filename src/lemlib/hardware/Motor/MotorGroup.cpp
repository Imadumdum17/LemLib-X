#include "hardware/Motor/MotorGroup.hpp"

#include <algorithm>
#include <cmath>
#include <climits>
#include <cstdint>

namespace {
int mergeResult(int current, std::int32_t next) { return std::max(current, static_cast<int>(next)); }
}

namespace lemlib_x {
MotorGroup::MotorGroup(std::initializer_list<ReversibleSmartPort> ports, AngularVelocity outputVelocity)
    : m_outputVelocity(outputVelocity) {
    for (const ReversibleSmartPort port : ports) { m_motors.push_back({port, false, 0_stDeg}); }
}

MotorGroup MotorGroup::from_pros_group(const pros::MotorGroup group, AngularVelocity outputVelocity) {
    MotorGroup result({}, outputVelocity);
    for (const std::int8_t port : group.get_port_all()) {
        result.addMotor(ReversibleSmartPort(port, runtime_check_port));
    }
    return result;
}

int MotorGroup::move(Number percent) {
    auto motors = getMotors();
    int result = 0;
    for (Motor& motor : motors) { result = mergeResult(result, motor.move(percent)); }
    return result;
}

int MotorGroup::moveVelocity(AngularVelocity velocity) {
    auto motors = getMotors();
    int result = 0;
    for (Motor& motor : motors) { result = mergeResult(result, motor.moveVelocity(velocity)); }
    return result;
}

int MotorGroup::brake() {
    auto motors = getMotors();
    int result = 0;
    for (Motor& motor : motors) { result = mergeResult(result, motor.brake()); }
    return result;
}

int MotorGroup::setBrakeMode(BrakeMode mode) {
    m_brakeMode = mode;
    auto motors = getMotors();
    int result = 0;
    for (Motor& motor : motors) { result = mergeResult(result, motor.setBrakeMode(mode)); }
    return result;
}

BrakeMode MotorGroup::getBrakeMode() { return m_brakeMode; }

int MotorGroup::isConnected() {
    auto motors = getMotors();
    for (Motor& motor : motors) {
        if (motor.isConnected() == 1) return 1;
    }
    return 0;
}

Angle MotorGroup::getAngle() {
    auto motors = getMotors();
    double sum = 0;
    int count = 0;
    for (Motor& motor : motors) {
        const Angle angle = motor.getAngle();
        if (std::isfinite(angle.internal())) {
            sum += angle.internal();
            count++;
        }
    }
    if (count == 0) return Angle(INFINITY);
    return Angle(sum / count);
}

int MotorGroup::setAngle(Angle angle) {
    int result = 0;
    for (MotorInfo& info : m_motors) {
        Motor motor(info.port, m_outputVelocity);
        result = mergeResult(result, motor.setAngle(angle));
        info.offset = motor.getOffset();
    }
    return result;
}

Current MotorGroup::getCurrentLimit() {
    Current sum = 0_amp;
    for (const Motor& motor : getMotors()) { sum += motor.getCurrentLimit(); }
    return sum;
}

int MotorGroup::setCurrentLimit(Current limit) {
    auto motors = getMotors();
    if (motors.empty()) return 0;
    int result = 0;
    const Current perMotorLimit = limit / static_cast<double>(motors.size());
    for (Motor& motor : motors) { result = mergeResult(result, motor.setCurrentLimit(perMotorLimit)); }
    return result;
}

std::vector<Temperature> MotorGroup::getTemperatures() {
    std::vector<Temperature> temperatures;
    for (const Motor& motor : getMotors()) { temperatures.push_back(motor.getTemperature()); }
    return temperatures;
}

int MotorGroup::setOutputVelocity(AngularVelocity outputVelocity) {
    m_outputVelocity = outputVelocity;
    return 0;
}

int MotorGroup::getSize() { return static_cast<int>(m_motors.size()); }

int MotorGroup::addMotor(ReversibleSmartPort port) {
    m_motors.push_back({port, false, 0_stDeg});
    return Motor(port, m_outputVelocity).setBrakeMode(m_brakeMode);
}

int MotorGroup::addMotor(Motor motor) { return addMotor(motor.getPort()); }

int MotorGroup::addMotor(Motor motor, bool reversed) { return addMotor(motor.getPort().set_reversed(reversed)); }

void MotorGroup::removeMotor(ReversibleSmartPort port) {
    const std::int8_t rawPort = static_cast<std::int8_t>(port);
    m_motors.erase(std::remove_if(m_motors.begin(), m_motors.end(),
                                  [rawPort](const MotorInfo& info) {
                                      return static_cast<std::int8_t>(info.port) == rawPort;
                                  }),
                   m_motors.end());
}

void MotorGroup::removeMotor(Motor motor) { removeMotor(motor.getPort()); }

Angle MotorGroup::configureMotor(ReversibleSmartPort port) {
    Motor motor(port, m_outputVelocity);
    motor.setBrakeMode(m_brakeMode);
    return motor.getAngle();
}

std::vector<Motor> MotorGroup::getMotors() {
    std::vector<Motor> motors;
    for (const MotorInfo& info : m_motors) {
        Motor& motor = motors.emplace_back(info.port, m_outputVelocity);
        motor.setOffset(info.offset);
    }
    return motors;
}
} // namespace lemlib_x
