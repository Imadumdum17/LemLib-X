/**
 * @file PAGE_SYSTEM_USAGE.md
 * @brief Guide for using LemLib's page system
 */

# Page System Usage Guide

## Overview
The page system allows you to create a multi-page display with touch navigation. It's ideal for displaying robot telemetry, settings menus, and other UI elements during matches.

## Basic Setup

### 1. Create Your Page Manager
Add this to your `opcontrol()` function:

```cpp
#include "lemlib/lemlib.hpp"

void opcontrol() {
    // Create page manager
    lemlib::PageManager pageManager;
    
    // Create pages
    auto homePage = std::make_shared<lemlib::HomePage>();
    auto telemetryPage = std::make_shared<lemlib::TelemetryPage>();
    auto settingsPage = std::make_shared<lemlib::SettingsPage>();
    
    // Add pages to manager
    pageManager.addPage(homePage);
    pageManager.addPage(telemetryPage);
    pageManager.addPage(settingsPage);
    
    // Add navigation buttons (adds to all pages)
    pageManager.addNavButton(50, 220, 150, 260, "Home", "Home");
    pageManager.addNavButton(250, 220, 350, 260, "Telemetry", "Telemetry");
    
    // Main control loop
    while (true) {
        pageManager.update();  // Update and render current page
        pros::delay(20);
    }
}
```

## Custom Pages

### Create a Custom Page
```cpp
#include "lemlib/Page.hpp"

class RobotStatusPage : public lemlib::Page {
public:
    RobotStatusPage() : Page("RobotStatus") {}
    
    void draw() override {
        // Draw your custom content
        pros::screen::set_pen(pros::c::COLOR_WHITE);
        pros::screen::print(pros::TEXT_LARGE_CENTER, 200, 20, "ROBOT STATUS");
        
        // Get data from your robot subsystems
        int motorTemp = chassis.getMotorTemp();  // Example
        pros::screen::print(pros::TEXT_MEDIUM, 10, 80, "Motor Temp: %dC", motorTemp);
    }
    
    void handleTouch(int16_t x, int16_t y, bool pressed) override {
        if (!pressed) return;
        // Handle custom touch inputs
    }
    
    void onEnter() override {
        pros::screen::erase();
    }
};
```

### Usage
```cpp
auto statusPage = std::make_shared<RobotStatusPage>();
pageManager.addPage(statusPage);
```

## Touch Navigation

### Adding Page Links
Create buttons that link to other pages:

```cpp
// Add navigation button
pageManager.addNavButton(
    50, 150,        // x0, y0 (top-left)
    150, 190,       // x1, y1 (bottom-right)
    "Telemetry",    // button label
    "Telemetry"     // target page name
);
```

### Handling Touch in Custom Pages
```cpp
void handleTouch(int16_t x, int16_t y, bool pressed) override {
    if (!pressed) return;  // Only handle press, not release
    
    // Check if touch is in a region
    if (x > 50 && x < 150 && y > 150 && y < 190) {
        // Handle click in this region
        ...
    }
}
```

## TouchButton Helper

The `TouchButton` struct provides:
- `contains(x, y)` - Check if coordinates are within button
- `draw()` - Render button with label
- `callback` - Function called when pressed

```cpp
lemlib::TouchButton btn;
btn.x0 = 50; btn.y0 = 100;
btn.x1 = 150; btn.y1 = 140;
btn.label = "Press Me";
btn.callback = []() { 
    // Do something when pressed
};
btn.draw();
```

## Lifecycle Hooks

Pages support enter/exit callbacks:

```cpp
void onEnter() override {
    // Called when page becomes active
    pros::screen::erase();
    // Reset any page state
}

void onExit() override {
    // Called when leaving the page
    // Clean up resources if needed
}
```

## Example: Real Robot Data

```cpp
class LiveTelemetryPage : public lemlib::Page {
public:
    LiveTelemetryPage(lemlib::Chassis* chassis) 
        : Page("LiveTelemetry"), chassis(chassis) {}
    
    void draw() override {
        pros::screen::set_pen(pros::c::COLOR_WHITE);
        
        auto pose = chassis->getPose();
        pros::screen::print(pros::TEXT_MEDIUM, 10, 60, 
            "X: %.2f Y: %.2f", pose.x, pose.y);
        
        pros::screen::print(pros::TEXT_MEDIUM, 10, 100, 
            "Theta: %.2f degrees", pose.theta);
    }
    
private:
    lemlib::Chassis* chassis;
};
```

## Tips

1. **Keep `draw()` fast** - Called every frame, minimize calculations
2. **Use `onEnter()` to clear screen** - Prevents ghost text from previous pages
3. **Debounce touches** - PageManager handles this automatically
4. **Test button positions** - Screen is 480x240 pixels
5. **Create reusable components** - Inherit from `Page` for common layouts

## Screen Dimensions
- Width: 480 pixels (0-480)
- Height: 240 pixels (0-240)

## Available Colors
All PROS colors are available: `pros::c::COLOR_WHITE`, `pros::c::COLOR_RED`, etc.
