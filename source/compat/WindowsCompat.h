#pragma once

#ifndef _WIN32
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "casepath.h"

#ifndef ZeroMemory
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif
#ifndef CopyMemory
#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#endif
#ifndef MoveMemory
#define MoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#endif
#include <wchar.h>
#include <wctype.h>
#include <stdarg.h>
#include <ctype.h>

// Basic Windows types mapped to portable stdint types
typedef unsigned long   DWORD;
typedef uint16_t       WORD;
typedef uint8_t        BYTE;
typedef int            BOOL;
typedef void*          HANDLE;
typedef void*          LPVOID;
typedef void*          HINSTANCE;
typedef void*          HWND;
typedef void*          HDC;
typedef void*          HBITMAP;
typedef void*          HRGN;
typedef void*          HKEY;
#define HKEY_CURRENT_USER  ((HKEY)(intptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(intptr_t)0x80000002)
typedef void*          HMODULE;
typedef void*          FARPROC;
typedef unsigned int   UINT;
typedef int            INT;
typedef long           LONG;
typedef unsigned long  ULONG;
typedef const char*    LPCSTR;
typedef char*          LPSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t*       LPWSTR;
typedef const char*    LPCTSTR;
typedef void*          HCURSOR;
typedef void*          HFONT;
typedef void*          HICON;
typedef void*          HMENU;
typedef uint32_t       COLORREF;

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define OutputDebugString(s) ((void)0)
#define OutputDebugStringA(s) ((void)0)
#define OutputDebugStringW(s) ((void)0)

static inline char* strupr(char* s) {
    if (!s) return NULL;
    for (char* p = s; *p; ++p) *p = (char)toupper((unsigned char)*p);
    return s;
}

static inline char* strlwr(char* s) {
    if (!s) return NULL;
    for (char* p = s; *p; ++p) *p = (char)tolower((unsigned char)*p);
    return s;
}
typedef char*          LPTSTR;
typedef int64_t        __int64;

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

typedef struct _WIN32_FIND_DATAA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    char     cFileName[260];
    char     cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;

#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_NORMAL    0x00000080

// BMP format headers
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

#define BI_RGB 0L

// String functions
#define stricmp  strcasecmp
#define strnicmp strncasecmp
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef pthread_mutex_t CRITICAL_SECTION;
typedef pthread_mutex_t* LPCRITICAL_SECTION;

static inline void InitializeCriticalSection(LPCRITICAL_SECTION cs) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(cs, &attr);
    pthread_mutexattr_destroy(&attr);
}

static inline void DeleteCriticalSection(LPCRITICAL_SECTION cs) {
    pthread_mutex_destroy(cs);
}

static inline void EnterCriticalSection(LPCRITICAL_SECTION cs) {
    pthread_mutex_lock(cs);
}

static inline void LeaveCriticalSection(LPCRITICAL_SECTION cs) {
    pthread_mutex_unlock(cs);
}

typedef uintptr_t WPARAM;
typedef intptr_t  LPARAM;
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID, *LPGUID;
typedef GUID IID;
typedef GUID CLSID;
typedef const GUID *REFGUID;
typedef const IID *REFIID;
typedef const CLSID *REFCLSID;

typedef uintptr_t UINT_PTR;
typedef intptr_t  LONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef void*     HGLOBAL;
typedef int32_t   HRESULT;
typedef intptr_t  LRESULT;
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef ANSI_CHARSET
#define ANSI_CHARSET 0

#ifndef MB_OK
#define MB_OK 0x00000000L
#define MB_OKCANCEL 0x00000001L
#define MB_YESNO 0x00000004L
#define MB_ICONERROR 0x00000010L
#define MB_ICONSTOP  0x00000010L
#define MB_ICONWARNING 0x00000030L
#define MB_ICONINFORMATION 0x00000040L
#define MB_APPLMODAL 0x00000000L
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7
#endif
#endif

static inline char* _strtime(char* buf) { if (buf) buf[0] = 0; return buf; }

#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
} MSG, *PMSG, *LPMSG;

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZE                         0x0005
#define WM_ACTIVATE                     0x0006
#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_ENABLE                       0x000A
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUIT                         0x0012
#define WM_QUERYOPEN                    0x0013
#define WM_SYSCOLORCHANGE               0x0015
#define WM_ACTIVATEAPP                  0x001C
#define WM_DISPLAYCHANGE                0x007E
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_TIMER                        0x0113
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A
#define WM_USER                         0x0400
#define HWND_BROADCAST                  ((HWND)0xffff)

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

static inline DWORD GetTickCount() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#include <sched.h>

static inline void Sleep(DWORD dwMilliseconds) {
    if (dwMilliseconds == 0) {
        sched_yield();
        return;
    }
    struct timespec req;
    req.tv_sec = dwMilliseconds / 1000;
    req.tv_nsec = (dwMilliseconds % 1000) * 1000000L;
    nanosleep(&req, NULL);
}

static inline DWORD GetCurrentThreadId() {
    return (DWORD)(uintptr_t)pthread_self();
}


#ifndef vsnwprintf
#define vsnwprintf vswprintf
#endif
#ifndef _vsnwprintf
#define _vsnwprintf vswprintf
#endif
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

static inline BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    lpPerformanceCount->QuadPart = (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
    return TRUE;
}

static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) {
    lpFrequency->QuadPart = 1000000000LL;
    return TRUE;
}

#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_ABOVE_NORMAL 1
static inline HANDLE GetCurrentThread() { return (HANDLE)(uintptr_t)pthread_self(); }
static inline int GetThreadPriority(HANDLE) { return 0; }
static inline BOOL SetThreadPriority(HANDLE, int) { return TRUE; }


static inline BOOL CreateCaret(HWND hWnd, HBITMAP hBitmap, int nWidth, int nHeight) { return TRUE; }
static inline BOOL ShowCaret(HWND hWnd) { return TRUE; }
static inline BOOL HideCaret(HWND hWnd) { return TRUE; }
static inline BOOL DestroyCaret() { return TRUE; }
static inline BOOL SetCaretPos(int X, int Y) { return TRUE; }


static inline DWORD timeGetTime() { return GetTickCount(); }
static inline void timeBeginPeriod(UINT uPeriod) {}
static inline void timeEndPeriod(UINT uPeriod) {}

#define SW_HIDE             0
#define SW_NORMAL           1
#define SW_SHOWNORMAL       1
#define SW_SHOWMINIMIZED    2
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWNA           7
#define SW_RESTORE          9

#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2

#define VK_RETURN           0x0D
#define SND_ASYNC           0x0001
static inline BOOL PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound) { return TRUE; }

#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern int gCompatCursorX;
extern int gCompatCursorY;
#ifdef __cplusplus
}
#endif

static inline BOOL ReleaseCapture() { return TRUE; }
static inline BOOL SetForegroundWindow(HWND hWnd) { return TRUE; }
static inline BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint) { return TRUE; }
static inline BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) { return TRUE; }
static inline BOOL GetCursorPos(LPPOINT lpPoint) {
    if (lpPoint) {
        lpPoint->x = gCompatCursorX;
        lpPoint->y = gCompatCursorY;
    }
    return TRUE;
}
static inline HWND WindowFromPoint(POINT Point) { return (HWND)1; }

typedef void (*_beginthread_proc_type)(void*);
struct _beginthread_arg {
    _beginthread_proc_type proc;
    void* arg;
};
static inline void* _beginthread_wrapper(void* p) {
    _beginthread_arg* a = (_beginthread_arg*)p;
    _beginthread_proc_type proc = a->proc;
    void* arg = a->arg;
    free(a);
    proc(arg);
    return NULL;
}
static inline uintptr_t _beginthread(_beginthread_proc_type start_address, unsigned stack_size, void* arglist) {
    pthread_t th;
    _beginthread_arg* a = (_beginthread_arg*)malloc(sizeof(_beginthread_arg));
    a->proc = start_address;
    a->arg = arglist;
    if (pthread_create(&th, NULL, _beginthread_wrapper, a) != 0) {
        free(a);
        return (uintptr_t)-1;
    }
    pthread_detach(th);
    return (uintptr_t)th;
}

#define SM_CXSCREEN         0
#define SM_CYSCREEN         1
#define SM_CXFULLSCREEN     0
#define SM_CYFULLSCREEN     1

static inline int GetSystemMetrics(int nIndex) {
    if (nIndex == SM_CXSCREEN || nIndex == SM_CXFULLSCREEN) return 640;
    if (nIndex == SM_CYSCREEN || nIndex == SM_CYFULLSCREEN) return 480;
    return 0;
}

static inline BOOL ShowWindow(HWND hWnd, int nCmdShow) { return TRUE; }
static inline HWND SetFocus(HWND hWnd) { return NULL; }
static inline BOOL DestroyWindow(HWND hWnd) { return TRUE; }
static inline UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, void* lpTimerFunc) { return nIDEvent; }
static inline BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent) { return TRUE; }
static inline BOOL PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return TRUE; }
static inline LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) { return 0; }
#define DefWindowProcA DefWindowProc
#define DefWindowProcW DefWindowProc
static inline HWND SetCapture(HWND hWnd) { return hWnd; }

#define VK_F1  0x70
#define VK_F2  0x71
#define VK_F3  0x72
#define VK_F4  0x73
#define VK_F5  0x74
#define VK_F6  0x75
#define VK_F7  0x76
#define VK_F8  0x77
#define VK_F9  0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B


#include <iostream>

#define GMEM_MOVEABLE 0x0002
#define GMEM_DDESHARE 0x2000
#define CF_TEXT        1
#define CF_BITMAP      2
#define CF_OEMTEXT     7
#define CF_UNICODETEXT 13
#define CF_LOCALE      16

#define CP_ACP         0
#define MB_PRECOMPOSED 0x00000001

static inline HGLOBAL GlobalAlloc(UINT uFlags, size_t dwBytes) { return malloc(dwBytes); }
static inline void*   GlobalLock(HGLOBAL hMem) { return hMem; }
static inline BOOL    GlobalUnlock(HGLOBAL hMem) { return TRUE; }
static inline HGLOBAL GlobalFree(HGLOBAL hMem) { free(hMem); return NULL; }

static inline BOOL OpenClipboard(HWND hWndNewOwner) { return TRUE; }
static inline BOOL CloseClipboard() { return TRUE; }
static inline HANDLE GetClipboardData(UINT uFormat) { return NULL; }
static inline HANDLE SetClipboardData(UINT uFormat, HANDLE hMem) { return hMem; }

static inline int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
    if (cbMultiByte < 0) cbMultiByte = (int)strlen(lpMultiByteStr) + 1;
    if (cchWideChar == 0) return cbMultiByte;
    for (int i = 0; i < cbMultiByte && i < cchWideChar; ++i)
        lpWideCharStr[i] = (wchar_t)(unsigned char)lpMultiByteStr[i];
    return cbMultiByte;
}

#define MB_ICONQUESTION 0x00000020L

static inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { return IDOK; }
static inline int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) { return IDOK; }
#ifndef MessageBox
#define MessageBox MessageBoxA
#endif

#define GWLP_USERDATA (-21)
#define GWL_USERDATA  (-21)
static inline LONG_PTR SetWindowLongPtr(HWND hWnd, int nIndex, LONG_PTR dwNewLong) { return 0; }
static inline LONG_PTR GetWindowLongPtr(HWND hWnd, int nIndex) { return 0; }
static inline LONG SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong) { return 0; }
static inline LONG GetWindowLong(HWND hWnd, int nIndex) { return 0; }

static inline BOOL DestroyCursor(HCURSOR hCursor) { return TRUE; }
static inline BOOL CloseHandle(HANDLE hObject) { return TRUE; }
static inline BOOL FreeLibrary(HMODULE hLibModule) { return TRUE; }

static inline BOOL IsIconic(HWND hWnd) { return FALSE; }
static inline BOOL IsWindowVisible(HWND hWnd) { return TRUE; }
static inline BOOL GetWindowRect(HWND hWnd, LPRECT lpRect) {
    if (lpRect) { lpRect->left = 0; lpRect->top = 0; lpRect->right = 640; lpRect->bottom = 480; }
    return TRUE;
}
static inline BOOL MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) { return TRUE; }

#define INFINITE      0xFFFFFFFF
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT  258
static inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) { return WAIT_OBJECT_0; }

static inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
    if (!lpFilename || nSize == 0) return 0;
    ssize_t len = readlink("/proc/self/exe", lpFilename, nSize - 1);
    if (len > 0) {
        lpFilename[len] = '\0';
        return (DWORD)len;
    }
    lpFilename[0] = 0;
    return 0;
}

static inline HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd) {
    return NULL;
}
static inline HINSTANCE ShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd) {
    return NULL;
}
#ifndef ShellExecute
#define ShellExecute ShellExecuteA
#endif

static inline HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) { return INVALID_HANDLE_VALUE; }
static inline BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) { return FALSE; }
static inline BOOL FindClose(HANDLE hFindFile) { return TRUE; }


#define THREAD_PRIORITY_NORMAL 0

static inline BOOL DeleteFileA(LPCSTR lpFileName) { return unlink(lpFileName) == 0; }
#ifndef DeleteFile
#define DeleteFile DeleteFileA
#endif
static inline char* GetCommandLineA() { static char sEmpty[] = ""; return sEmpty; }
static inline LPWSTR GetCommandLineW() { static wchar_t sEmpty[] = L""; return sEmpty; }
#ifndef GetCommandLine
#define GetCommandLine GetCommandLineA
#endif

static inline BOOL EnumWindows(void* lpEnumFunc, LPARAM lParam) { return TRUE; }
static inline LONG ChangeDisplaySettings(void* lpDevMode, DWORD dwFlags) { return 0; }
static inline BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, void* pvParam, UINT fWinIni) { return TRUE; }
#define SPI_GETSCREENSAVETIMEOUT 14
#define SPI_GETSCREENSAVEACTIVE  16

typedef struct tagWINDOWPLACEMENT {
    UINT length;
    UINT flags;
    UINT showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT rcNormalPosition;
} WINDOWPLACEMENT;
static inline BOOL GetWindowPlacement(HWND hWnd, WINDOWPLACEMENT* lpwndpl) { return TRUE; }

#define KEY_READ                0x20019
#define KEY_WRITE               0x20006
#define KEY_ALL_ACCESS          0xF003F
#define REG_OPTION_NON_VOLATILE 0
#define REG_SZ                  1
#define REG_BINARY              3
#define REG_DWORD               4
#define ERROR_SUCCESS           0L

static inline LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult) { return 1; }
static inline LONG RegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, DWORD samDesired, void* lpSecurityAttributes, HKEY* phkResult, DWORD* lpdwDisposition) { return 1; }
static inline LONG RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) { return 1; }
static inline LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, DWORD* lpReserved, DWORD* lpType, BYTE* lpData, DWORD* lpcbData) { return 1; }
static inline LONG RegDeleteKeyA(HKEY hKey, LPCSTR lpSubKey) { return 1; }
static inline LONG RegDeleteValueA(HKEY hKey, LPCSTR lpValueName) { return 1; }
static inline LONG RegEnumKeyA(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName) { return 1; }
static inline LONG RegCloseKey(HKEY hKey) { return 0; }

#endif // !_WIN32
