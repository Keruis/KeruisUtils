#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <Windows.h>

#include "../Script/ScriptObject.h"
#include "../Script/ClassRegistry.h"

class WindowController : public ScriptObject {
public:
    explicit WindowController();

    void setWindowTitle(std::string title);
    bool findWindow();
    bool isVisible() const;
    bool setTopMost(bool enable);

private:
    std::string m_windowTitle;
    HWND m_hwnd;
};

REGISTER_CLASS(WindowController);

#endif //WINDOWCONTROLLER_H
