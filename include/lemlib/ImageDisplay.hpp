#pragma once

#include "pros/screen.hpp"
#include <string>
#include <vector>

namespace lemlib_x {

/**
 * @brief Simple image/bitmap display utility for the brain screen
 * 
 * Provides easy methods to display images on the V5 brain screen
 */
class ImageDisplay {
public:
    /**
     * @brief Create an image display manager
     */
    ImageDisplay() = default;

    /**
     * @brief Display an image from a file at specified position
     * 
     * @param filename Path to the image file (PNG, JPG, etc.)
     * @param x X coordinate (0-479)
     * @param y Y coordinate (0-239)
     * @return true if image was displayed successfully
     * 
     * @code {.cpp}
     * lemlib_x::ImageDisplay imageDisplay;
     * imageDisplay.displayImage("usd/logo.png", 200, 100);
     * @endcode
     */
    bool displayImage(const std::string& filename, int16_t x, int16_t y);

    /**
     * @brief Display a centered image on the screen
     * 
     * @param filename Path to the image file
     * @return true if image was displayed successfully
     */
    bool displayImageCentered(const std::string& filename);

    /**
     * @brief Display an image that fills the entire screen
     * 
     * @param filename Path to the image file
     * @return true if image was displayed successfully
     */
    bool displayImageFullscreen(const std::string& filename);

    /**
     * @brief Create a simple rectangle filled with color
     * 
     * @param x0 Top-left X
     * @param y0 Top-left Y
     * @param x1 Bottom-right X
     * @param y1 Bottom-right Y
     * @param color Color to fill with
     */
    void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);

    /**
     * @brief Create a simple circle
     * 
     * @param x Center X
     * @param y Center Y
     * @param radius Circle radius
     * @param color Color to draw
     */
    void drawCircle(int16_t x, int16_t y, int16_t radius, uint32_t color);

    /**
     * @brief Draw text on screen
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param text Text to display
     * @param color Text color
     * @param size Font size (1-4, where 1 is smallest)
     */
    void drawText(int16_t x, int16_t y, const std::string& text, uint32_t color, uint16_t size = 2);

    /**
     * @brief Clear the screen with a background color
     * 
     * @param bgColor Background color (default black)
     */
    void clearScreen(uint32_t bgColor = pros::c::COLOR_BLACK);

    /**
     * @brief Get the width of the screen in pixels
     * @return Screen width (480)
     */
    static constexpr int16_t getScreenWidth() { return 480; }

    /**
     * @brief Get the height of the screen in pixels
     * @return Screen height (240)
     */
    static constexpr int16_t getScreenHeight() { return 240; }

private:
    // Helper to validate coordinates
    bool isValidCoordinate(int16_t x, int16_t y) const;
};

} // namespace lemlib_x
