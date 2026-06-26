#include "hardware/IMU/V5InertialSensor.hpp"

#include <cmath>
#include <cstdint>

namespace lemlib_x {
V5InertialSensor::V5InertialSensor(SmartPort port)
    : m_imu(static_cast<std::uint8_t>(port)) {}

V5InertialSensor V5InertialSensor::from_pros_imu(pros::Imu imu) {
    return V5InertialSensor(SmartPort(imu.get_port(), runtime_check_port));
}

int V5InertialSensor::calibrate() { return static_cast<int>(m_imu.reset(false)); }

int V5InertialSensor::isCalibrated() { return !isCalibrating(); }

int V5InertialSensor::isCalibrating() { return m_imu.is_calibrating() ? 1 : 0; }

int V5InertialSensor::isConnected() { return m_imu.is_installed() ? 1 : 0; }

Angle V5InertialSensor::getRotation() {
    const double rotation = m_imu.get_rotation();
    if (!std::isfinite(rotation)) return Angle(INFINITY);
    return from_stDeg(rotation) * m_gyroScalar + m_offset;
}

int V5InertialSensor::setRotation(Angle rotation) {
    m_offset = rotation - (from_stDeg(m_imu.get_rotation()) * m_gyroScalar);
    return static_cast<int>(m_imu.set_rotation(to_stDeg(rotation)));
}

int V5InertialSensor::setGyroScalar(Number scalar) {
    m_gyroScalar = scalar;
    return 0;
}

Number V5InertialSensor::getGyroScalar() { return m_gyroScalar; }

int IMU::setGyroScalar(Number scalar) {
    m_gyroScalar = scalar;
    return 0;
}

Number IMU::getGyroScalar() { return m_gyroScalar; }
} // namespace lemlib_x
