#ifndef LEMLIB_EXAMPLE_PAGES_HPP
#define LEMLIB_EXAMPLE_PAGES_HPP

#include "lemlib/Page.hpp"
#include "lemlib/PageManager.hpp"

namespace lemlib_x {

/**
 * @brief Example home/menu page
 */
class HomePage : public Page {
public:
    HomePage();
    void draw() override;
    void handleTouch(int16_t x, int16_t y, bool pressed) override;
    void onEnter() override;

private:
    std::vector<TouchButton> buttons;
};

/**
 * @brief Example telemetry/status page
 */
class TelemetryPage : public Page {
public:
    TelemetryPage();
    void draw() override;
    void handleTouch(int16_t x, int16_t y, bool pressed) override;
    void onEnter() override;

private:
    // Add your telemetry variables here
    // Example: int driveMotorTemp = 0;
    std::vector<TouchButton> buttons;
};

/**
 * @brief Example settings/config page
 */
class SettingsPage : public Page {
public:
    SettingsPage();
    void draw() override;
    void handleTouch(int16_t x, int16_t y, bool pressed) override;
    void onEnter() override;

private:
    int selectedOption = 0;
    std::vector<TouchButton> buttons;
};

} // namespace lemlib_x

#endif // LEMLIB_EXAMPLE_PAGES_HPP
