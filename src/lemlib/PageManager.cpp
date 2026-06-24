#include "lemlib/PageManager.hpp"
#include "pros/screen.hpp"

namespace lemlib_x {

PageManager::PageManager() : activePage(nullptr) {}

void PageManager::addPage(std::shared_ptr<Page> page) {
    if (!page) return;
    
    std::string pageName = page->getName();
    pages[pageName] = page;
    
    // Set first page as active
    if (!activePage) {
        activePage = page;
        activePageName = pageName;
        activePage->onEnter();
    }
}

void PageManager::switchPage(const std::string& pageName) {
    if (!pageExists(pageName)) {
        return; // Page doesn't exist
    }
    
    // Don't switch if already on this page
    if (activePageName == pageName) {
        return;
    }
    
    // Call exit handler on current page
    if (activePage) {
        activePage->onExit();
    }
    
    // Switch to new page
    activePage = pages[pageName];
    activePageName = pageName;
    
    // Call enter handler
    activePage->onEnter();
    
    // Clear screen and redraw
    pros::screen::erase();
}

std::shared_ptr<Page> PageManager::getActivePage() const {
    return activePage;
}

std::string PageManager::getActivePageName() const {
    return activePageName;
}

void PageManager::update() {
    if (!activePage) return;
    
    // Handle touch input
    auto touchStatus = pros::screen::touch_status();
    bool currentPressed = touchStatus.touch_status == pros::E_TOUCH_PRESSED || 
                          touchStatus.touch_status == pros::E_TOUCH_HELD;
    
    // On touch press event
    if (currentPressed && !lastTouchPressed) {
        activePage->handleTouch(touchStatus.x, touchStatus.y, true);
        
        // Check navigation buttons for this page
        if (pageButtons.count(activePageName)) {
            for (auto& button : pageButtons[activePageName]) {
                if (button.contains(touchStatus.x, touchStatus.y)) {
                    button.callback();
                    break;
                }
            }
        }
    }
    
    // On touch release event
    if (!currentPressed && lastTouchPressed) {
        activePage->handleTouch(touchStatus.x, touchStatus.y, false);
    }
    
    lastTouchPressed = currentPressed;
    
    // Draw the page
    activePage->draw();
}

void PageManager::addNavButton(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               const std::string& label, const std::string& targetPage,
                               const std::string* pageName) {
    if (!pageExists(targetPage)) {
        return; // Target page doesn't exist
    }
    
    TouchButton button;
    button.x0 = x0;
    button.y0 = y0;
    button.x1 = x1;
    button.y1 = y1;
    button.label = label;
    button.callback = [this, targetPage]() {
        this->switchPage(targetPage);
    };
    
    if (pageName == nullptr) {
        // Add button to all pages
        for (auto& page : pages) {
            pageButtons[page.first].push_back(button);
        }
    } else {
        // Add button to specific page
        pageButtons[*pageName].push_back(button);
    }
}

size_t PageManager::getPageCount() const {
    return pages.size();
}

bool PageManager::pageExists(const std::string& pageName) const {
    return pages.count(pageName) > 0;
}

} // namespace lemlib_x
