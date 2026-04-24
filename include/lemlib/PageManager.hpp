#ifndef LEMLIB_PAGE_MANAGER_HPP
#define LEMLIB_PAGE_MANAGER_HPP

#include "Page.hpp"
#include <memory>
#include <map>
#include <vector>

namespace lemlib {

/**
 * @brief Manages pages and handles page switching with touch navigation
 * 
 * The PageManager controls which page is active, handles touch events,
 * and manages the update/draw loop.
 */
class PageManager {
public:
    /**
     * @brief Construct a new PageManager
     */
    PageManager();

    /**
     * @brief Add a page to the manager
     * @param page Shared pointer to the page to add
     */
    void addPage(std::shared_ptr<Page> page);

    /**
     * @brief Switch to a page by name
     * @param pageName The name of the page to switch to
     */
    void switchPage(const std::string& pageName);

    /**
     * @brief Get the currently active page
     * @return Pointer to active page, nullptr if none active
     */
    std::shared_ptr<Page> getActivePage() const;

    /**
     * @brief Get the name of the active page
     * @return Name of active page
     */
    std::string getActivePageName() const;

    /**
     * @brief Update and render the current page
     * 
     * Should be called in your main control loop.
     * Handles touch input and page drawing.
     */
    void update();

    /**
     * @brief Add a navigation button that switches pages
     * 
     * @param x0, y0 Top-left corner of button
     * @param x1, y1 Bottom-right corner of button
     * @param label Button label text
     * @param targetPage Page name to switch to
     * @param pageName Optional: add button only to specific page (nullptr = all pages)
     */
    void addNavButton(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                      const std::string& label, const std::string& targetPage,
                      const std::string* pageName = nullptr);

    /**
     * @brief Get number of registered pages
     */
    size_t getPageCount() const;

    /**
     * @brief Check if a page exists by name
     */
    bool pageExists(const std::string& pageName) const;

private:
    std::map<std::string, std::shared_ptr<Page>> pages;
    std::shared_ptr<Page> activePage;
    std::string activePageName;
    
    // Navigation buttons per page
    std::map<std::string, std::vector<TouchButton>> pageButtons;
    
    // Last touch state for debouncing
    bool lastTouchPressed = false;
};

} // namespace lemlib

#endif // LEMLIB_PAGE_MANAGER_HPP
