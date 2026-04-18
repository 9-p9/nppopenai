#pragma once
// Stub for src/ui/dialogs/LoaderDlg.h – used only to satisfy extern declaration
// in external_globals.h; no Windows API calls needed in tests.
#include <string>

class LoaderDlg
{
public:
    LoaderDlg() = default;
    void setModelName(const std::wstring &) {}
    void show(bool = false) {}
    void hide() {}
    bool isVisible() const { return false; }
    void resetDialog() {}
    bool isCancelled() const { return false; }
};
