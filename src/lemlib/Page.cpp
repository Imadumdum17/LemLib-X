#include "lemlib/Page.hpp"
#include "pros/screen.hpp"

namespace lemlib {

Page::Page(const std::string& name) : name(name) {}

void TouchButton::draw() const {
    // Draw button border
    pros::screen::set_pen(pros::c::COLOR_WHITE);
    pros::screen::draw_rect(x0, y0, x1, y1);
    
    // Draw button label centered
    int16_t centerX = (x0 + x1) / 2;
    int16_t centerY = (y0 + y1) / 2;
    pros::screen::print(pros::E_TEXT_MEDIUM_CENTER, centerX, centerY, "%s", label.c_str());
}

} // namespace lemlib
