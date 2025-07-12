#include "WindowController.h"

WindowController::WindowController() {
    m_hwnd = nullptr;

    registerMethod("setWindowTitle", [this](const std::vector<std::any>& args) -> std::any {
        setWindowTitle(std::any_cast<std::string>(args[0]));
        return {};
    });

    registerMethod("setTopMost", [this](const std::vector<std::any>& args) -> std::any {
        setTopMost(std::any_cast<bool>(args[0]));
        return {};
    });
}

void WindowController::setWindowTitle(std::string title) {
    m_windowTitle = title;
}

bool WindowController::findWindow() {
    m_hwnd = FindWindowA(nullptr, m_windowTitle.c_str());
    return m_hwnd != nullptr;
}

bool WindowController::isVisible() const {
    return m_hwnd != nullptr && IsWindow(m_hwnd);
}

bool WindowController::setTopMost(bool enable) {
    if (!isVisible()) {
        return false;
    }

    return SetWindowPos(m_hwnd,
                            enable ? HWND_TOPMOST : HWND_NOTOPMOST,
                            0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}