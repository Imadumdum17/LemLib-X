#ifndef LEMLIB_PAGE_HPP
#define LEMLIB_PAGE_HPP

#include "pros/screen.hpp"
#include <string>
#include <functional>
#include <vector>

namespace lemlib {

/**
 * @brief Base class for display pages
 * 
 * Inherit from this class to create custom pages for your display system.
 * Pages handle their own rendering and touch interactions.
 */
class Page {
public:
    /**
     * @brief Construct a new Page
     * @param name The name/identifier of this page
     */
    explicit Page(const std::string& name);

    virtual ~Page() = default;

    /**
     * @brief Draw/update the page content
     * 
     * Called whenever the page needs to be rendered.
     * Implement this to draw your page content.
     */
    virtual void draw() = 0;

    /**
     * @brief Handle screen touch events
     * 
     * @param x X coordinate of touch
     * @param y Y coordinate of touch
     * @param pressed Whether the screen was pressed (true) or released (false)
     */
    virtual void handleTouch(int16_t x, int16_t y, bool pressed) {}

    /**
     * @brief Called when page becomes active
     */
    virtual void onEnter() {}

    /**
     * @brief Called when page is about to become inactive
     */
    virtual void onExit() {}

    /**
     * @brief Get the name of this page
     * @return The page name
     */
    std::string getName() const { return name; }

    /**
     * @brief Clear screen with specified background color
     * @param bgColor Background color (default black)
     */
    void clearScreen(uint32_t bgColor = pros::c::COLOR_BLACK) {
        pros::screen::set_eraser(bgColor);
        pros::screen::erase();
    }

protected:
    std::string name;
};

/**
 * @brief Struct representing a touch button on screen
 */
struct TouchButton {
    int16_t x0, y0;     ///< Top-left corner
    int16_t x1, y1;     ///< Bottom-right corner
    std::string label;  ///< Button label
    std::function<void()> callback; ///< Function to call when pressed

    /**
     * @brief Check if coordinates fall within button bounds
     */
    bool contains(int16_t x, int16_t y) const {
        return x >= x0 && x <= x1 && y >= y0 && y <= y1;
    }

    /**
     * @brief Draw the button on screen
     */
    void draw() const;
};

} // namespace lemlib

#endif // LEMLIB_PAGE_HPP
