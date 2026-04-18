#pragma once
// Stub for src/ui/dialogs/DockingDlgInterface.h – minimal class skeleton for test compilation.
#include "StaticDialog.h"

class DockingDlgInterface : public StaticDialog
{
public:
    DockingDlgInterface() = default;
    explicit DockingDlgInterface(int dlgID) : _dlgID(dlgID) {}
    virtual ~DockingDlgInterface() = default;
    virtual void init(HINSTANCE hInst, HWND parent) { Window::init(hInst, parent); }

protected:
    int          _dlgID      = 0;
    std::wstring _moduleName;
    virtual INT_PTR CALLBACK run_dlgProc(UINT, WPARAM, LPARAM) override { return 0; }
};
