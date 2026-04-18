#pragma once
// Stub for src/ui/dialogs/StaticDialog.h – minimal class skeleton for test compilation.
#include "Window.h"

typedef long (WINAPI *ETDTProc)(HWND, DWORD);

enum class PosAlign { left, right, top, bottom };

struct DLGTEMPLATEEX {
    WORD dlgVer; WORD signature; DWORD helpID;
    DWORD exStyle; DWORD style; WORD cDlgItems;
    short x; short y; short cx; short cy;
};

class StaticDialog : public Window
{
public:
    virtual ~StaticDialog() = default;
    virtual void create(int /*dialogID*/, bool /*isRTL*/ = false, bool /*msgDestParent*/ = true) {}
    virtual bool isCreated() const { return (_hSelf != nullptr); }
    void    goToCenter() {}
    virtual void display(bool /*toShow*/ = true, bool /*enhanced*/ = false) const {}
    virtual void destroy() override {}

protected:
    RECT _rc = {};
    static INT_PTR CALLBACK dlgProc(HWND, UINT, WPARAM, LPARAM) { return 0; }
    virtual INT_PTR CALLBACK run_dlgProc(UINT, WPARAM, LPARAM) = 0;
};
