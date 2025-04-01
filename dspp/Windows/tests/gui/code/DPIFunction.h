#include <iostream>
#include <Windows.h>
#include <cassert>

// Copied for IMGUI
typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);


// Platform headers
static BOOL _IsWindowsVersionOrGreater(WORD major, WORD minor, WORD)
{
	typedef LONG(WINAPI* PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);
	static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfoFn = nullptr;
	if (RtlVerifyVersionInfoFn == nullptr)
		if (HMODULE ntdllModule = ::GetModuleHandleA("ntdll.dll"))
			RtlVerifyVersionInfoFn = (PFN_RtlVerifyVersionInfo)GetProcAddress(ntdllModule, "RtlVerifyVersionInfo");
	if (RtlVerifyVersionInfoFn == nullptr)
		return FALSE;

	RTL_OSVERSIONINFOEXW versionInfo = { };
	ULONGLONG conditionMask = 0;
	versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	versionInfo.dwMajorVersion = major;
	versionInfo.dwMinorVersion = minor;
	VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	return (RtlVerifyVersionInfoFn(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) == 0) ? TRUE : FALSE;
}

#define OverWin8() _IsWindowsVersionOrGreater(HIBYTE(0x0603), LOBYTE(0x0603), 0) // _WIN32_WINNT_WINBLUE

	extern float getDPIScale(void* hwnd) {
		HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
		UINT xdpi = 96, ydpi = 96;
		if (OverWin8())
		{
			static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
			static PFN_GetDpiForMonitor GetDpiForMonitorFn = nullptr;
			if (GetDpiForMonitorFn == nullptr && shcore_dll != nullptr)
				GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor");
			if (GetDpiForMonitorFn != nullptr)
			{
				GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
				//			PASSERT(xdpi == ydpi); // Please contact me if you hit this assert!
				if (xdpi == 96.0f) return 2.0f;
				return xdpi / 96.0f;
			}
		}
#ifndef NOGDI
		const HDC dc = ::GetDC(nullptr);
		xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
		ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
		assert(xdpi == ydpi); // Please contact me if you hit this assert!
		::ReleaseDC(nullptr, dc);
#endif
		return xdpi / 96.0f;
	}
