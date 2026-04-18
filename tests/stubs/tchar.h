#pragma once
// Stub for <tchar.h> – only needed on non-Windows builds.
#ifndef _WIN32
#  include "windows.h"   // ensure TCHAR = wchar_t is defined
#  define _T(x)    L##x
#  define _tcslen  wcslen
#  define _tcscpy  wcscpy
#  define _tcscat  wcscat
#  define _tcscmp  wcscmp
#  define _tcsncmp wcsncmp
#  define _tcschr  wcschr
#  define _tcsstr  wcsstr
#endif
