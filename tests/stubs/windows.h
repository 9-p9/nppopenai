#pragma once
// Minimal Windows API stubs for cross-platform (non-Windows) compilation.
// Used only in the test build to allow compiling Windows plugin source files on Linux.

#ifndef _WIN32

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cwchar>

// ---- Basic Windows types ----
typedef wchar_t   WCHAR;
typedef wchar_t   TCHAR;
typedef unsigned char  UCHAR;
typedef unsigned short WORD;
typedef unsigned int   UINT;
typedef unsigned long  DWORD;
typedef long           LONG;
typedef unsigned long long ULONGLONG;
typedef long long      LONGLONG;
typedef int            BOOL;
typedef uintptr_t      UINT_PTR;
typedef uintptr_t      ULONG_PTR;
typedef intptr_t       LONG_PTR;
typedef ULONG_PTR      SIZE_T;
typedef LONG_PTR       SSIZE_T;
typedef DWORD          COLORREF;

// Handles (opaque pointers on non-Windows)
typedef void* HWND;
typedef void* HANDLE;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void* HICON;
typedef void* HMENU;
typedef void* HBRUSH;
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HRGN;
typedef void* HPEN;
typedef void* HFONT;
typedef void* HCURSOR;
typedef void* HGLOBAL;
typedef void* DLGTEMPLATE;

// Message and result types
typedef long           LRESULT;
typedef UINT_PTR       WPARAM;
typedef LONG_PTR       LPARAM;
typedef int            INT_PTR;
typedef LONG           HRESULT;

// Structures
struct RECT  { long left, top, right, bottom; };
struct POINT { long x, y; };
struct SIZE  { long cx, cy; };

// Calling conventions / declspec (empty on non-Windows)
#define CALLBACK
#define WINAPI
#define WINAPIV
#define APIENTRY
#define PASCAL
#define __cdecl
#define __declspec(x)
#define __forceinline inline

// Common constants
#ifndef NULL
#  define NULL nullptr
#endif
#define TRUE    1
#define FALSE   0
#define MAX_PATH 260
#define INFINITE 0xFFFFFFFF
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

// ShowWindow / visibility flags
#define SW_SHOW    5
#define SW_HIDE    0
#define SW_SHOWNA  8

// SetWindowPos flags
#define SWP_NOMOVE     0x0002
#define SWP_NOSIZE     0x0001
#define SWP_SHOWWINDOW 0x0040
#define SWP_NOACTIVATE 0x0010
#define HWND_TOPMOST   ((HWND)(LONG_PTR)-1)

// WM_ messages
#define WM_USER    0x0400
#define WM_TIMER   0x0113
#define WM_COMMAND 0x0111
#define WM_INITDIALOG 0x0110
#define WM_CLOSE   0x0010
#define WM_DESTROY 0x0002

// Button message constants
#define BM_GETCHECK    0x00F0
#define BM_SETCHECK    0x00F1
#define BST_UNCHECKED  0x0000
#define BST_CHECKED    0x0001

// Text / unicode
#ifdef _UNICODE
#  define TEXT(x) L##x
#else
#  define TEXT(x) x
#endif

// COM helpers
#define S_OK   ((HRESULT)0L)
#define FAILED(hr) ((HRESULT)(hr) < 0)

// CP constants
#define CP_UTF8 65001
#define CP_ACP  0
#define MB_ERR_INVALID_CHARS 0x00000008

// ---- Minimal inline stubs for Windows API functions ----

inline int MultiByteToWideChar(UINT, DWORD, const char* src, int srcLen,
                                wchar_t* dst, int dstLen)
{
    if (!src) return 0;
    int needed = 0;
    for (int i = 0; (srcLen < 0 ? src[i] : i < srcLen); ++i) needed++;
    if (srcLen < 0) needed++; // null terminator
    if (!dst || dstLen == 0) return needed;
    int i = 0;
    while (i < dstLen - 1 && (srcLen < 0 ? src[i] != '\0' : i < srcLen)) {
        dst[i] = static_cast<wchar_t>(static_cast<unsigned char>(src[i]));
        ++i;
    }
    dst[i] = L'\0';
    return i + 1;
}

inline int WideCharToMultiByte(UINT, DWORD, const wchar_t*, int,
                                char*, int, const char*, BOOL*) { return 0; }

inline bool IsWindowVisible(HWND) { return false; }
inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT SendDlgItemMessage(HWND, int, UINT, WPARAM, LPARAM) { return 0; }
inline BOOL ShowWindow(HWND, int)  { return FALSE; }
inline BOOL UpdateWindow(HWND)     { return FALSE; }
inline BOOL InvalidateRect(HWND, const RECT*, BOOL) { return FALSE; }
inline BOOL SetForegroundWindow(HWND) { return FALSE; }
inline BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return FALSE; }
inline BOOL MoveWindow(HWND, int, int, int, int, BOOL) { return FALSE; }
inline HWND GetDlgItem(HWND, int)  { return nullptr; }
inline BOOL SetDlgItemText(HWND, int, const wchar_t*) { return FALSE; }
inline UINT_PTR SetTimer(HWND, UINT_PTR, UINT, void*) { return 0; }
inline BOOL KillTimer(HWND, UINT_PTR) { return FALSE; }
inline ULONGLONG GetTickCount64() { return 0ULL; }
inline BOOL GetClientRect(HWND, RECT* rc) { if (rc) *rc = {0,0,0,0}; return FALSE; }
inline BOOL GetWindowRect(HWND, RECT* rc) { if (rc) *rc = {0,0,0,0}; return FALSE; }
inline HWND SetFocus(HWND) { return nullptr; }
inline int MessageBox(HWND, const wchar_t*, const wchar_t*, UINT) { return 0; }
inline BOOL PostMessage(HWND, UINT, WPARAM, LPARAM) { return FALSE; }
inline BOOL DestroyWindow(HWND) { return FALSE; }

inline HMODULE GetModuleHandle(const char*) { return nullptr; }
inline DWORD GetModuleFileName(HMODULE, wchar_t* buf, DWORD len)
    { if (buf && len > 0) buf[0] = L'\0'; return 0; }

inline DWORD GetPrivateProfileString(const wchar_t*, const wchar_t*,
                                     const wchar_t* def, wchar_t* out,
                                     DWORD sz, const wchar_t*)
{
    if (out && sz > 0 && def) {
        DWORD i = 0;
        while (def[i] && i < sz - 1) { out[i] = def[i]; ++i; }
        out[i] = L'\0';
    }
    return 0;
}
inline BOOL WritePrivateProfileString(const wchar_t*, const wchar_t*,
                                      const wchar_t*, const wchar_t*) { return FALSE; }

#endif // !_WIN32
