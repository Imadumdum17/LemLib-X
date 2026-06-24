#include "hardware/Encoder/ADIEncoder.hpp"

#include "hardware/Port.hpp"
#include "pros/adi.hpp"
#include "pros/error.h"
#include "units/Angle.hpp"
#include <cmath>
#include <climits>

namespace lemlib_x {
ADIEncoder::ADIEncoder(pros::adi::Encoder encoder) : m_encoder(std::move(encoder)) {}

ADIEncoder::ADIEncoder(ADIPort topPort, ADIPort bottomPort, bool reversed)
    : m_encoder(static_cast<std::uint8_t>(topPort), static_cast<std::uint8_t>(bottomPort), reversed) {}

ADIEncoder::ADIEncoder(SmartPort expanderPort, ADIPort topPort, ADIPort bottomPort, bool reversed)
    : m_encoder({static_cast<std::uint8_t>(expanderPort), static_cast<std::uint8_t>(topPort),
                 static_cast<std::uint8_t>(bottomPort)}, reversed) {}

int ADIEncoder::isConnected() {
    const std::int32_t value = m_encoder.get_value();
    if (value == PROS_ERR) {
        return INT_MAX;
    }
    return 1;
}

Angle ADIEncoder::getAngle() {
    const std::int32_t value = m_encoder.get_value();
    if (value == PROS_ERR) {
        return Angle(INFINITY);
    }
    return from_stRot(static_cast<double>(value) / 360.0) + m_offset;
}

int ADIEncoder::setAngle(Angle angle) {
    const std::int32_t value = m_encoder.get_value();
    if (value == PROS_ERR) {
        return INT_MAX;
    }
    m_offset = angle - from_stRot(static_cast<double>(value) / 360.0);
    return 0;
}
} // namespace lemlib_x
