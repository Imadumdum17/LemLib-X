#include "hardware/Encoder/V5RotationSensor.hpp"

#include "hardware/Port.hpp"
#include "pros/error.h"
#include "pros/rotation.hpp"
#include "units/Angle.hpp"
#include <cmath>
#include <climits>

namespace lemlib_x {
V5RotationSensor::V5RotationSensor(ReversibleSmartPort port)
    : m_rotation(static_cast<std::int8_t>(port)),
      m_reversed(port.is_reversed()),
      m_port(static_cast<int>(port)) {}

V5RotationSensor V5RotationSensor::from_pros_rot(pros::Rotation encoder) {
    return V5RotationSensor(ReversibleSmartPort(encoder.get_port(), runtime_check_port));
}

int V5RotationSensor::isConnected() {
    if (m_port <= 0) {
        return 0;
    }

    const std::int32_t result = m_rotation.get_angle();
    if (result == PROS_ERR) {
        return INT_MAX;
    }
    return 1;
}

Angle V5RotationSensor::getAngle() {
    const std::int32_t rawAngle = m_rotation.get_angle();
    if (rawAngle == PROS_ERR) {
        return Angle(INFINITY);
    }
    return from_stDeg(static_cast<double>(rawAngle) / 100.0) + m_offset;
}

int V5RotationSensor::setAngle(Angle angle) {
    const std::int32_t rawAngle = m_rotation.get_angle();
    if (rawAngle == PROS_ERR) {
        return INT_MAX;
    }
    m_offset = angle - from_stDeg(static_cast<double>(rawAngle) / 100.0);
    return 0;
}

int V5RotationSensor::isReversed() const { return static_cast<int>(m_reversed); }

int V5RotationSensor::setReversed(bool reversed) {
    const std::int32_t result = m_rotation.set_reversed(reversed);
    if (result == PROS_ERR) {
        return INT_MAX;
    }
    m_reversed = reversed;
    return 0;
}
} // namespace lemlib_x
