#pragma once
// Stub for <shlwapi.h> – only PathFindFileName is used in the plugin headers.
#ifndef _WIN32
#  include "windows.h"
inline const wchar_t* PathFindFileName(const wchar_t* path)
{
    if (!path) return path;
    const wchar_t* p = path;
    while (*p) {
        if (*p == L'/' || *p == L'\\') path = p + 1;
        ++p;
    }
    return path;
}
#endif
