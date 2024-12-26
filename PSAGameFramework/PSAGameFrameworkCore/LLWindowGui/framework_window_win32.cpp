// framework_window_win32.
#include "framework_window.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_WINDOW_WIN32 {
	/* windows api, microsoft docs & chatgpt.
	* ���� windows δ���� api.
	* Ŀǰ���ṩģ��ƽ̨�����Ա���.
	* RCSZ (20241226).
	*/
#ifdef _WIN32
	// ���崰���������ö��.
	enum WINDOWCOMPOSITIONATTRIB {
		WCA_UNDEFINED     = 0,
		WCA_ACCENT_POLICY = 19,
		WCA_BLURBEHIND    = 20,
	};

	// define(wapi) 'ACCENT_STATE'.
	enum ACCENT_STATE {
		ACCENT_DISABLED                   = 0,
		ACCENT_ENABLE_GRADIENT            = 1,
		ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
		ACCENT_ENABLE_BLURBEHIND          = 3, // ë����Ч��
		ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4, // �ǿ���Ч��
		ACCENT_ENABLE_HOSTBACKDROP        = 5,
	};

	struct ACCENT_POLICY {
		ACCENT_STATE AccentState;
		int AccentFlags;
		int GradientColor;
		int AnimationId;
	};
	
	struct WINDOWCOMPOSITIONATTRIBDATA {
		WINDOWCOMPOSITIONATTRIB DataAttribute;
		PVOID  DataPtrVoid;
		SIZE_T DataSize;
	};

	// �ֶ����� SetWindowCompositionAttribute ����.
	using pSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

	// �������ڱ���ģ��Ч��.
	bool EnableWindowsBlur(HWND hwnd) {
		// ��̬���� "user32.dll" �е� SetWindowCompositionAttribute ����.
		auto hModule = LoadLibrary(TEXT("user32.dll"));
		if (!hModule) {
			PushLogger(LogError, PSAG_WIN32_LABEL, "failed load 'user32.dll' library.");
			return false;
		}
		auto SetWindowCompositionAttribute = reinterpret_cast<pSetWindowCompositionAttribute>(
			GetProcAddress(hModule, "SetWindowCompositionAttribute")
		);
		if (!SetWindowCompositionAttribute) {
			PushLogger(
				LogError, PSAG_WIN32_LABEL, "failed get '%s' function_ptr.",
				"SetWindowCompositionAttribute"
			);
			FreeLibrary(hModule);
			return false;
		}
		// ���� ACCENT_POLICY Ϊë����Ч��.
		ACCENT_POLICY Accent = {};
		Accent.AccentState   = ACCENT_ENABLE_BLURBEHIND; // ë����Ч��.
		Accent.AccentFlags   = 1;                        // ����ģ���ĳ̶�. (1,2 �ǳ���ֵ)
		Accent.GradientColor = 0xAA00FF00;               // ��͸����.

		// ACCENT_POLICY => WINDOW.
		WINDOWCOMPOSITIONATTRIBDATA data = {};
		data.DataAttribute = WCA_ACCENT_POLICY;
		data.DataPtrVoid   = &Accent;
		data.DataSize      = sizeof(Accent);

		SetWindowCompositionAttribute(hwnd, &data);
		FreeLibrary(hModule);
		return true;
	}

	// ����͸����ʽ.
	void EnableWindowsTransparency(HWND hwnd) {
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
	}

	inline COLORREF SystemConvertColorRGB(const ImVec4& color) {
		return RGB(int(255.0f * color.x), int(255.0f * color.y), int(255.0f * color.z));
	}
	// windows dwm api, win7? win10, win11.
	void SetSystemWindowTitlebarColor(GLFWwindow* window, const ImVec4& border) {
		HWND Hwnd = glfwGetWin32Window(window);
		BOOL DarkMode = TRUE;

		COLORREF Titlebar = SystemConvertColorRGB(border);

		DwmSetWindowAttribute(Hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &DarkMode, sizeof(DarkMode));
		DwmSetWindowAttribute(Hwnd, DWMWA_BORDER_COLOR, &Titlebar, sizeof(Titlebar));
		DwmSetWindowAttribute(Hwnd, DWMWA_CAPTION_COLOR, &Titlebar, sizeof(Titlebar));

		COLORREF TitleConstColor = RGB(0, 255, 255);
		DwmSetWindowAttribute(Hwnd, DWMWA_TEXT_COLOR, &TitleConstColor, sizeof(TitleConstColor));
	}

	bool MemoryIsOnHeap(void* ptr) {
		return HeapValidate(GetProcessHeap(), NULL, ptr);
	}

	size_t MemoryHeapSizeBytes(void* ptr) {
		return HeapSize(GetProcessHeap(), NULL, ptr);
	}

	ProcessMemoryInfo CurrentProcMemoryInfo() {
		ProcessMemoryInfo ResultTemp = {};
		ResultTemp.ProcessHandle = GetCurrentProcess();

		PROCESS_MEMORY_COUNTERS _PMC;
		GetProcessMemoryInfo(ResultTemp.ProcessHandle, &_PMC, sizeof(_PMC));

		ResultTemp.PhyMemory = _PMC.WorkingSetSize;
		ResultTemp.VirMemory = _PMC.PagefileUsage;
		// h_p physical virtual memory, c_version: 101, RCSZ.
		return ResultTemp;
	}
#endif
}