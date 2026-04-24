#include "lemlib/ExamplePages.hpp"
#include "pros/screen.hpp"

namespace lemlib {

// ============== HOME PAGE ==============

HomePage::HomePage() : Page("Home") {
    // Create navigation buttons
    buttons.push_back({50, 150, 150, 190, "Telemetry", {}});
    buttons.push_back({250, 150, 350, 190, "Settings", {}});
}

void HomePage::draw() {
    // Draw title
    pros::screen::set_pen(pros::c::COLOR_WHITE);
    pros::screen::print(pros::E_TEXT_LARGE_CENTER, 200, 20, "HOME");
    
    // Draw border line
    pros::screen::draw_line(0, 50, 480, 50);
    
    // Draw buttons
    for (auto& button : buttons) {
        button.draw();
    }
    
    // Draw home status info
    pros::screen::set_pen(pros::c::COLOR_WHITE);
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, 80, "Pages: %d", 3);
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, 110, "Touch buttons below");
}

void HomePage::handleTouch(int16_t x, int16_t y, bool pressed) {
    if (!pressed) return;
    
    for (auto& button : buttons) {
        if (button.contains(x, y)) {
            // These callbacks will be set by PageManager
            if (button.callback) {
                button.callback();
            }
            break;
        }
    }
}

void HomePage::onEnter() {
    clearScreen();
}

// ============== TELEMETRY PAGE ==============

TelemetryPage::TelemetryPage() : Page("Telemetry") {
    // Create navigation buttons
    buttons.push_back({50, 220, 150, 260, "Home", {}});
    buttons.push_back({250, 220, 350, 260, "Settings", {}});
}

void TelemetryPage::draw() {
    // Draw title
    pros::screen::set_pen(pros::c::COLOR_WHITE);
    pros::screen::print(pros::E_TEXT_LARGE_CENTER, 200, 20, "TELEMETRY");
    
    // Draw border line
    pros::screen::draw_line(0, 50, 480, 50);
    
    // Display telemetry data
    // Example data - replace with your actual robot telemetry
    int lineY = 70;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Drive Motor: 45C");
    lineY += 30;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Battery: 12.5V");
    lineY += 30;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Position: X:0.5 Y:1.2");
    lineY += 30;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Heading: 45.3 deg");
    lineY += 30;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Status: Ready");
    
    // Draw navigation buttons
    for (auto& button : buttons) {
        button.draw();
    }
}

void TelemetryPage::handleTouch(int16_t x, int16_t y, bool pressed) {
    if (!pressed) return;
    
    for (auto& button : buttons) {
        if (button.contains(x, y)) {
            if (button.callback) {
                button.callback();
            }
            break;
        }
    }
}

void TelemetryPage::onEnter() {
    clearScreen();
}

// ============== SETTINGS PAGE ==============

SettingsPage::SettingsPage() : Page("Settings") {
    // Create navigation buttons
    buttons.push_back({50, 220, 150, 260, "Home", {}});
    buttons.push_back({250, 220, 350, 260, "Telemetry", {}});
}

void SettingsPage::draw() {
    // Draw title
    pros::screen::set_pen(pros::c::COLOR_WHITE);
    pros::screen::print(pros::E_TEXT_LARGE_CENTER, 200, 20, "SETTINGS");
    
    // Draw border line
    pros::screen::draw_line(0, 50, 480, 50);
    
    // Display settings
    int lineY = 80;
    pros::screen::set_pen(pros::c::COLOR_WHITE);
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Option 1: Disabled");
    lineY += 40;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Option 2: Enabled");
    lineY += 40;
    pros::screen::print(pros::E_TEXT_MEDIUM, 10, lineY, "Sensitivity: High");
    
    // Draw navigation buttons
    for (auto& button : buttons) {
        button.draw();
    }
}

void SettingsPage::handleTouch(int16_t x, int16_t y, bool pressed) {
    if (!pressed) return;
    
    for (auto& button : buttons) {
        if (button.contains(x, y)) {
            if (button.callback) {
                button.callback();
            }
            break;
        }
    }
}

void SettingsPage::onEnter() {
    clearScreen();
}

} // namespace lemlib
