#pragma once
// Stub for src/ui/dialogs/Window.h – minimal class skeleton for test compilation.
#include <windows.h>

class Window
{
public:
    Window() = default;
    Window(const Window &) = delete;
    virtual ~Window() = default;

    virtual void init(HINSTANCE hInst, HWND parent)
    {
        _hInst   = hInst;
        _hParent = parent;
    }
    virtual void destroy() {}
    virtual void display(bool /*toShow*/ = true) const {}
    virtual bool isVisible() const { return false; }
    HWND getHSelf()   const { return _hSelf; }
    HWND getHParent() const { return _hParent; }
    HINSTANCE getHinst() const { return _hInst; }
    Window &operator=(const Window &) = delete;

protected:
    HINSTANCE _hInst   = nullptr;
    HWND      _hParent = nullptr;
    HWND      _hSelf   = nullptr;
};
