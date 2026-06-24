#include "lemlib/ImageDisplay.hpp"
#include "pros/screen.hpp"

namespace lemlib_x {

bool ImageDisplay::displayImage(const std::string& filename, int16_t x, int16_t y) {
    if (!isValidCoordinate(x, y)) {
        return false;
    }
    
    pros::screen::print(pros::text_format_e_t::E_TEXT_LARGE, x, y, "%s", filename.c_str());
    // Note: Direct image display requires PROS API support
    // This is a placeholder - actual implementation depends on PROS version
    return true;
}

bool ImageDisplay::displayImageCentered(const std::string& filename) {
    int16_t centerX = (getScreenWidth() / 2);
    int16_t centerY = (getScreenHeight() / 2);
    
    // Assume standard image is ~100x100, adjust center accordingly
    return displayImage(filename, centerX - 50, centerY - 50);
}

bool ImageDisplay::displayImageFullscreen(const std::string& filename) {
    return displayImage(filename, 0, 0);
}

void ImageDisplay::drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color) {
    if (!isValidCoordinate(x0, y0) || !isValidCoordinate(x1, y1)) {
        return;
    }
    
    pros::screen::set_pen(color);
    pros::screen::draw_rect(x0, y0, x1, y1);
}

void ImageDisplay::drawCircle(int16_t x, int16_t y, int16_t radius, uint32_t color) {
    if (!isValidCoordinate(x, y)) {
        return;
    }
    
    pros::screen::set_pen(color);
    pros::screen::draw_circle(x, y, radius);
}

void ImageDisplay::drawText(int16_t x, int16_t y, const std::string& text, uint32_t color, uint16_t size) {
    if (!isValidCoordinate(x, y)) {
        return;
    }
    
    // Clamp size between 1 and 4
    uint16_t clampedSize = size < 1 ? 1 : (size > 4 ? 4 : size);
    
    pros::screen::set_pen(color);
    
    // PROS uses different formats for different sizes
    pros::text_format_e_t format;
    if (clampedSize == 1) {
        format = pros::text_format_e_t::E_TEXT_SMALL;
    } else if (clampedSize == 2) {
        format = pros::text_format_e_t::E_TEXT_MEDIUM;
    } else if (clampedSize == 3) {
        format = pros::text_format_e_t::E_TEXT_LARGE;
    } else {
        format = pros::text_format_e_t::E_TEXT_LARGE;
    }
    
    pros::screen::print(format, x, y, "%s", text.c_str());
}

void ImageDisplay::clearScreen(uint32_t bgColor) {
    pros::screen::set_eraser(bgColor);
    pros::screen::erase();
}

bool ImageDisplay::isValidCoordinate(int16_t x, int16_t y) const {
    return (x >= 0 && x < getScreenWidth() && y >= 0 && y < getScreenHeight());
}

} // namespace lemlib_x
