// framework_window_win32.
#include "framework_window.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_WINDOW_WIN32 {
	/* windows api, microsoft docs & chatgpt.
	* 包含 windows 未公布 api.
	* 目前不提供模块平台兼容性保障.
	* RCSZ (20241226).
	*/
#ifdef _WIN32
	// 定义窗口组合属性枚举.
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
		ACCENT_ENABLE_BLURBEHIND          = 3, // 毛玻璃效果
		ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4, // 亚克力效果
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

	// 手动加载 SetWindowCompositionAttribute 函数.
	using pSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

	// 开启窗口背景模糊效果.
	bool EnableWindowsBlur(HWND hwnd) {
		// 动态加载 "user32.dll" 中的 SetWindowCompositionAttribute 函数.
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
		// 配置 ACCENT_POLICY 为毛玻璃效果.
		ACCENT_POLICY Accent = {};
		Accent.AccentState   = ACCENT_ENABLE_BLURBEHIND; // 毛玻璃效果.
		Accent.AccentFlags   = 1;                        // 控制模糊的程度. (1,2 是常用值)
		Accent.GradientColor = 0xAA00FF00;               // 半透明黑.

		// ACCENT_POLICY => WINDOW.
		WINDOWCOMPOSITIONATTRIBDATA data = {};
		data.DataAttribute = WCA_ACCENT_POLICY;
		data.DataPtrVoid   = &Accent;
		data.DataSize      = sizeof(Accent);

		SetWindowCompositionAttribute(hwnd, &data);
		FreeLibrary(hModule);
		return true;
	}

	// 开启透明样式.
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