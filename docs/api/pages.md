# Page System - Tuning UI Documentation

## Overview

The Page System provides an interactive on-robot UI for real-time tuning of PID gains, exit conditions, and testing motions without recompiling. This is essential for efficient development and competition preparation.

## Architecture

```
┌─────────────────────────────────────┐
│         PageManager                 │
│   (Manages page navigation)         │
└──────────────┬──────────────────────┘
               │
        ┌──────▼──────┐
        │ Page Stack  │
        └──────┬──────┘
               │
    ┌──────────┼──────────┐
    │          │          │
    ▼          ▼          ▼
 [Tuning]  [Testing]  [Custom]
   Page       Page       Pages
```

## Core Components

### Page Base Class

```cpp
class Page {
    public:
        virtual ~Page() = default;
        
        // Called when page becomes active
        virtual void onEnter() {}
        
        // Called each frame (10ms)
        virtual void update() = 0;
        
        // Called when user navigates away
        virtual void onExit() {}
        
        // Render to screen
        virtual void render() = 0;
};
```

### PageManager

Manages page navigation and lifecycle.

```cpp
class PageManager {
    public:
        // Push new page onto stack
        void push(Page* page);
        
        // Pop current page
        void pop();
        
        // Update current page
        void update();
        
        // Render current page
        void render();
        
        // Check if specific page is active
        bool isActive(Page* page) const;
};
```

## Built-in Pages

### PID Tuning Page

Real-time adjustment of PID gains during testing.

```cpp
class PIDTuningPage : public Page {
    public:
        // Create tuning page for specific PID controller
        PIDTuningPage(lemlib::PID& pid, const char* name);
        
        void update() override;
        void render() override;
};
```

**Usage:**

```cpp
// In your main code
void openTuningUI() {
    static lemlib::PIDTuningPage tuningPage(lateral_pid, "Lateral PID");
    pageManager.push(&tuningPage);
}
```

**Controls:**
- **Up/Down arrows**: Select parameter (kP, kI, kD)
- **Left/Right arrows**: Adjust value
- **Button A**: Save values
- **Button B**: Return to previous menu

### Exit Condition Tuning Page

Adjust motion settling conditions interactively.

```cpp
class ExitConditionPage : public Page {
    public:
        ExitConditionPage(lemlib::ExitCondition<Length>& condition,
                         const char* name);
        
        void update() override;
        void render() override;
};
```

**Usage:**

```cpp
// Tune exit conditions without recompiling
static lemlib::ExitConditionPage lateralPage(
    lateral_exit_conditions[0],  // First exit condition
    "Lateral Exit"
);
```

### Motion Test Page

Execute pre-configured motions via UI.

```cpp
class MotionTestPage : public Page {
    public:
        MotionTestPage();
        
        // Add motion to menu
        void addMotion(const char* name, std::function<void()> motion);
        
        void update() override;
        void render() override;
};
```

**Usage:**

```cpp
void setupMotionTests() {
    static lemlib::MotionTestPage tests;
    
    // Add tests
    tests.addMotion("Move 12in", []() {
        lemlib::moveToPoint(Point(12, 0), 3000_msec);
    });
    
    tests.addMotion("Turn 90°", []() {
        lemlib::turnTo(90_deg, 2000_msec);
    });
    
    tests.addMotion("Complex", []() {
        lemlib::moveToPoint(Point(24, 0), 5000_msec);
        while (lemlib::motion_handler::isMoving()) pros::delay(10);
        
        lemlib::turnTo(90_deg, 2000_msec);
        while (lemlib::motion_handler::isMoving()) pros::delay(10);
    });
    
    pageManager.push(&tests);
}
```

### Telemetry Page

Real-time display of robot state.

```cpp
class TelemetryPage : public Page {
    public:
        void update() override {
            // Displays:
            // - Current pose
            // - Motor voltages
            // - IMU heading
            // - Motion status
        }
        
        void render() override;
};
```

**Usage:**

```cpp
static lemlib::TelemetryPage telemetry;
pageManager.push(&telemetry);

// Displays:
// Position: (24.5, 36.2)
// Heading: 45.3°
// Motors: L=8000V R=8000V
// Motion: Running
```

## Custom Pages

Create specialized UI pages for your needs.

### Simple Info Page

```cpp
class InfoPage : public lemlib::Page {
    private:
        const char* m_title;
        
    public:
        InfoPage(const char* title) : m_title(title) {}
        
        void update() override {
            // Handle input
            if (controller.get_digital_new_press(DIGITAL_A)) {
                // Button A pressed
            }
        }
        
        void render() override {
            // Draw to screen
            pros::lcd::set_background(COLOR_WHITE);
            pros::lcd::print(0, m_title);
            pros::lcd::print(1, "Select an option:");
            pros::lcd::print(2, "A) Continue");
            pros::lcd::print(3, "B) Back");
        }
};
```

### Menu Page

```cpp
class MenuPage : public lemlib::Page {
    private:
        std::vector<std::pair<const char*, std::function<void()>>> m_options;
        int m_selected = 0;
        
    public:
        void addOption(const char* label, std::function<void()> action) {
            m_options.push_back({label, action});
        }
        
        void update() override {
            // Navigate menu
            if (controller.get_digital_new_press(DIGITAL_UP)) {
                m_selected--;
                if (m_selected < 0) m_selected = m_options.size() - 1;
            }
            if (controller.get_digital_new_press(DIGITAL_DOWN)) {
                m_selected++;
                if (m_selected >= m_options.size()) m_selected = 0;
            }
            
            // Execute selected option
            if (controller.get_digital_new_press(DIGITAL_A)) {
                m_options[m_selected].second();
            }
        }
        
        void render() override {
            pros::lcd::clear();
            for (size_t i = 0; i < m_options.size(); i++) {
                const char* marker = (i == m_selected) ? ">" : " ";
                pros::lcd::print(i, "%s %s", marker, m_options[i].first);
            }
        }
};
```

## Integration Example

### Complete UI Setup

```cpp
// File: src/main.cpp

#include "main.h"
#include "lemlib/api.hpp"

// Global page manager
lemlib::PageManager pageManager;

// Pages
lemlib::PIDTuningPage lateralTuning(lateral_pid, "Lateral PID");
lemlib::PIDTuningPage angularTuning(angular_pid, "Angular PID");
lemlib::TelemetryPage telemetry;

// Custom menu page
class MainMenuPage : public lemlib::Page {
    private:
        int m_selected = 0;
        
    public:
        void update() override {
            if (controller.get_digital_new_press(DIGITAL_UP)) {
                m_selected = (m_selected - 1 + 3) % 3;
            }
            if (controller.get_digital_new_press(DIGITAL_DOWN)) {
                m_selected = (m_selected + 1) % 3;
            }
            
            if (controller.get_digital_new_press(DIGITAL_A)) {
                switch (m_selected) {
                    case 0:
                        pageManager.push(&lateralTuning);
                        break;
                    case 1:
                        pageManager.push(&angularTuning);
                        break;
                    case 2:
                        pageManager.push(&telemetry);
                        break;
                }
            }
        }
        
        void render() override {
            pros::lcd::clear();
            pros::lcd::print(0, "=== TUNING MENU ===");
            pros::lcd::print(1, "%s Tune Lateral PID", m_selected == 0 ? ">" : " ");
            pros::lcd::print(2, "%s Tune Angular PID", m_selected == 1 ? ">" : " ");
            pros::lcd::print(3, "%s Telemetry", m_selected == 2 ? ">" : " ");
        }
};

static MainMenuPage mainMenu;

void setupUI() {
    pageManager.push(&mainMenu);
}

void opcontrol() {
    // Initialize UI
    setupUI();
    
    while (true) {
        // Tank drive
        int leftPower = controller.get_analog(ANALOG_LEFT_Y);
        int rightPower = controller.get_analog(ANALOG_RIGHT_Y);
        left_motors.moveVoltage(leftPower * 120);
        right_motors.moveVoltage(rightPower * 120);
        
        // Update odometry
        odom.update();
        
        // Update UI
        pageManager.update();
        pageManager.render();
        
        pros::delay(10);
    }
}
```

## Workflow: PID Tuning via UI

### Step 1: Enter Tuning UI

```
Robot boots → Main Menu → Select "Tune Lateral PID"
```

### Step 2: Adjust Parameters

```
Lateral PID Tuning
Current kP: 0.8 <-- Selected
Current kI: 0.05
Current kD: 0.10

Up/Down: Select parameter
Left/Right: Adjust value ±0.1
A: Apply
B: Back
```

### Step 3: Test Motion

1. Exit tuning UI
2. Run autonomous motion
3. Observe behavior
4. Return to tuning UI
5. Adjust if needed
6. Repeat

### Step 4: Save Configuration

Once tuned, update `src/main.cpp`:

```cpp
// Update with tuned values
lemlib::PID lateral_pid(0.85, 0.05, 0.12);  // New tuned values
```

## Advanced: Logger Page

Display logged data in UI.

```cpp
class LoggerPage : public lemlib::Page {
    private:
        std::vector<std::string> m_logs;
        int m_offset = 0;
        
    public:
        void log(const char* message) {
            m_logs.push_back(message);
        }
        
        void update() override {
            if (controller.get_digital_new_press(DIGITAL_UP)) {
                m_offset = std::max(0, m_offset - 1);
            }
            if (controller.get_digital_new_press(DIGITAL_DOWN)) {
                m_offset++;
            }
        }
        
        void render() override {
            pros::lcd::clear();
            for (int i = 0; i < 8; i++) {
                int idx = m_offset + i;
                if (idx < m_logs.size()) {
                    pros::lcd::print(i, m_logs[idx].c_str());
                }
            }
        }
};
```

## Best Practices

1. **Test changes in UI first** - before committing to code
2. **Document your menu structure** - for team reference
3. **Use clear button labels** - make UI intuitive
4. **Add confirmation dialogs** - for important changes
5. **Save tuned values** - once working well
6. **Version control pages** - track tuning history
7. **Test on field** - UI tuning ≠ real-world tuning

## Screen Resolution

V5 Robot Brain screen:
- **Resolution**: 480×240 pixels
- **Text lines**: ~8 lines at standard text size
- **Update rate**: 10 ms refresh

## Performance Considerations

### Keep Updates Quick

```cpp
void update() override {
    // ✓ Fast - just check input
    if (controller.get_digital_new_press(DIGITAL_A)) {
        doSomething();
    }
    
    // ✗ Slow - heavy computation here
    // Don't do expensive loops in update()
}
```

### Efficient Rendering

```cpp
void render() override {
    // ✓ Clear once
    pros::lcd::clear();
    
    // ✓ Print only what changed
    static int lastValue = -1;
    if (currentValue != lastValue) {
        pros::lcd::print(0, "Value: %d", currentValue);
        lastValue = currentValue;
    }
}
```

## Troubleshooting

### Issue: Page doesn't update

**Solution:** Make sure `pageManager.update()` and `pageManager.render()` are called in opcontrol loop

### Issue: Controller input not responsive

**Solution:** Check that controller reading is prioritized before page updates

### Issue: Screen too cluttered

**Solution:** Use multiple pages instead of rendering everything at once

## See Also

- [MotionHandler](./api/motion-handler.md) - Running motions from UI
- [Configuration](./api/configuration.md) - Storing tuned values
- [PID Controller](./api/pid.md) - Understanding what you're tuning
- [ExitCondition](./api/exit-condition.md) - Motion settling tuning
