// framework_window.
#define ENABLE_GLFW_NATIVE_WIN32

#ifndef _FRAMEWORK_WINDOW_H
#define _FRAMEWORK_WINDOW_H
#include <GLFW/glfw3.h>
#ifdef ENABLE_GLFW_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#define PSAG_IMGUI_CONTROL_MATHS
#include "framework_window_imgui_ms.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(_MSV_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib,"legacy_stdio_definitons.lib")
#endif

#ifdef _WIN32
// windows platform api.
#include <windows.h>
#include <psapi.h>
#include <dwmapi.h>
// load system library.
#pragma comment(lib, "Dwmapi.lib")
#endif

// LLWindowGui PSA-L.6 (=>logger).
#include "../LLLogger/framework_logger.hpp"

#define DEVICE_ENABLE_NVIDIA_GPU

namespace PSAG_WINDOW_OGLFW {
	StaticStrLABEL PSAG_WINDOW_LABEL = "PSAG_WINDOW";

	// glfw window init_config. 
	struct FwWindowConfig {
		std::string WindowName;

		uint32_t WindowSizeWidth, WindowSizeHeight;
		bool WindowFullFlag;

		FwWindowConfig() :
			WindowSizeWidth (768),
			WindowSizeHeight(432),
			WindowName      ({}),
			WindowFullFlag  (false)
		{}
		FwWindowConfig(uint32_t w, uint32_t h, const std::string& name, bool fs) :
			WindowSizeWidth (w),
			WindowSizeHeight(h),
			WindowName      (name),
			WindowFullFlag  (fs)
		{}
	};

	class PsagGLFWsystemCALLBACK {
	protected:
		static Vector3T<float> ValueMouseScroll; // x:pos, y:min, z:max
		static Vector2T<float> ValueMouseCursor; // x:pos.x, y:pos.y
		static bool            ValueWindowFocus; // true: focused
		static Vector2T<float> ValueWindowSize;  // x:width, y:height
		// window refresh event function.
		static std::function<void(std::chrono::steady_clock::time_point)> REFRESH_CALL_FUNC;

		static std::vector<std::string> FilePaths; // => "CallbackDropFile"

		static void CallbackScroll     (GLFWwindow* window, double xoffset, double yoffset);
		static void CallbackCursorPos  (GLFWwindow* window, double xpos, double ypos);
		static void CallbackFocus      (GLFWwindow* window, int focus);
		static void CallbackDropFiles  (GLFWwindow* window, int count, const char** paths);
		static void CallbackWindowSize (GLFWwindow* window, int width, int height);

		static void CallbackWindowClose  (GLFWwindow* window);
		static void CallbackWindowRefresh(GLFWwindow* window);

#ifdef DEVICE_ENABLE_WINAPI_PROCESS
		static WNDPROC OLD_WINDOW_PROC;
		static HWND    OLD_WINDOW_HWND;
#endif
	};

	using FwSysErrorMessage = const char*;
	// core-framework inherits window-event.
	class PasgWindowEvent :public PsagGLFWsystemCALLBACK {
	private:
		static std::chrono::steady_clock::time_point FrameTimer;
		static float CalcFrameTime;
		static float CalcStepTimeTemp[2]; // 0:speed, 1:smooth.
	protected:
		GLFWwindow* MainWindowObject = {};

		Vector2T<int32_t> RenderBufferSize = Vector2T<int>(32, 32);
		Vector4T<float>   RenderClearColor = Vector4T<float>();

		FwSysErrorMessage ErrorMessage = nullptr;

		bool GLFWwindowCreate(FwWindowConfig GenNoiseConfig = {});
		bool GLFWwindowInit(uint32_t version[2], uint32_t MSAA, bool core_mode, bool fixedsize);

		void GLFWwindowSetIcon(const std::string& file);

		bool GLFWwindowVsync(bool vswitch);
		bool GLFWwindowCallback();

		bool GLFWwindowFree();

		void RenderContextAbove(); // opengl render context above.
		void RenderContextBelow(); // opengl render context below.

		void  ClacFrameTimeStepBegin();
		float CalcFrameTimeStepEnd(float base_fps);
		// return true:close.
		bool CloseFlag() { return (bool)glfwWindowShouldClose(MainWindowObject); };
	};
}

#ifdef ENABLE_PSAG_IMGUI_MATHS

ImVec2 PsagConvertVec2ToImVec2(const Vector2T<float>& vec) { return ImVec2(vec.vector_x, vec.vector_y); }
ImVec4 PsagConvectVec4ToImVec4(const Vector4T<float>& vec) { return ImVec4(vec.vector_x, vec.vector_y, vec.vector_z, vec.vector_w); }

Vector2T<float> PsagConvertImVec2ToVec2(const ImVec2& vec) { return Vector2T<float>(vec.x, vec.y); }
Vector4T<float> PsagConvectImVec4ToVec4(const ImVec4& vec) { return Vector4T<float>(vec.x, vec.y, vec.z, vec.w); }

ImU32 PsagConvertVec4ToImU32(const Vector4T<float> color) {
	return ImGui::ColorConvertFloat4ToU32(PsagConvectVec4ToImVec4(color));
}
#define PSAG_DEFCOLOR_RED    0xFF0000FF // 红色
#define PSAG_DEFCOLOR_GREEN  0x00FF00FF // 绿色
#define PSAG_DEFCOLOR_BLUE   0x0000FFFF // 蓝色
#define PSAG_DEFCOLOR_CYAN   0x00FFFFFF // 青色
#define PSAG_DEFCOLOR_YELLOW 0xFFFF00FF // 黄色
#define PSAG_DEFCOLOR_WHITE  0xFFFFFFFF // 白色
#endif

namespace PSAG_WINDOW_IMGUI {
	StaticStrLABEL PSAG_IMGUI_LABEL = "PSAG_IMGUI";

	struct ImGuiConfig {
		std::string     ShaderVersionStr;              // opengl glsl version string
		std::string     FontsFilepath;                 // imgui: fonts global read_fIlepath'.ttf'
		Vector4T<float> FontsGlobalColor;              // imgui: fonts global_style_color
		float           FontsGlobalSize;               // imgui: fonts global_style_sizescale
		float           FrameRounding, WindowRounding; // imgui: frame,window global_style_rounding

		ImGuiConfig() :
			ShaderVersionStr({}),
			FontsFilepath   ({}),
			FontsGlobalColor({}),
			FontsGlobalSize (16.0f),
			FrameRounding   (0.0f),
			WindowRounding  (0.0f)
		{}
		ImGuiConfig(
			const std::string& version, const std::string& path, const Vector4T<float>& color, 
			float size, float round, float wround
		) :
			ShaderVersionStr(version),
			FontsFilepath   (path),
			FontsGlobalColor(color),
			FontsGlobalSize (size),
			FrameRounding   (round),
			WindowRounding  (wround)
		{}
	};

	class PsagImGuiContextEvent {
	protected:
		bool ImGuiContextInit(GLFWwindow* window_object, ImGuiConfig im_config = {});
		void ImGuiContextFree();

		void RenderGuiContextA();
		void RenderGuiContextB();
	};
}

namespace PSAG_WINDOW_WIN32 {
#ifdef _WIN32
	StaticStrLABEL PSAG_WIN32_LABEL = "PSAG_WIN32";

	bool EnableWindowsBlur(HWND hwnd);
	void EnableWindowsTransparency(HWND hwnd);

	// windows api + opengl glfw api.
	void SetSystemWindowTitlebarColor(GLFWwindow* window, const ImVec4& border);

	bool   MemoryIsOnHeap     (void* ptr);
	size_t MemoryHeapSizeBytes(void* ptr);

	struct ProcessMemoryInfo {
		HANDLE ProcessHandle = {};

		size_t PhyMemory = NULL;
		size_t VirMemory = NULL;
	};
	// 获取当前进程内存信息.[v101]
	ProcessMemoryInfo CurrentProcMemoryInfo();
#endif
}

namespace ImPsag {
	void DebugGuiFlagStatus();
	bool GetDebugGuiFlag();

	inline ImVec2 TO_IMVEC2(const Vector2T<float>& vec2) {
		return ImVec2(vec2.vector_x, vec2.vector_y);
	}
	// color(r,g,b,a): value => r,g,b ahpla => a.
	ImVec4 ColorGrayscaleScale(const ImVec4& color, float value, float ahpla = 0.0f);

	float ItemCenteredCalcX(float width);
	float ItemCenteredCalcY(float height);

	void ListDrawLine(const ImVec2& point0, const ImVec2& point1, const ImVec4& color, float width);
	void ListDrawText(const ImVec2& position, const ImVec4& color, const char* text, ...);

	void ListDrawRectangle    (const ImVec2& position, const ImVec2& size, const ImVec4& color, float wline = 3.2f);
	void ListDrawRectangleFill(const ImVec2& position, const ImVec2& size, const ImVec4& color);
	void ListDrawCircle       (const ImVec2& position, float size, const ImVec4& color, float wline = 3.2f);
	void ListDrawCircleFill   (const ImVec2& position, float size, const ImVec4& color);
	void ListDrawTriangle     (const ImVec2& position, const ImVec2& offset1, const ImVec2& offset2, const ImVec4& color, float wline = 3.2f);
	void ListDrawTriangleFill (const ImVec2& position, const ImVec2& offset1, const ImVec2& offset2, const ImVec4& color);

	void ListDrawRoundImage(const ImVec2& position, const ImVec2& size, ImTextureID texture);

	void ListDrawPolyLine(std::vector<ImVec2>* vertices, const ImVec4& color, float wline = 3.2f);
	void ListDrawPolyFilled(std::vector<ImVec2>* vertices, const ImVec4& color);

	namespace ImAnim {
#define ANIM_STD_STEP_BUTTON 0.078f
		// chlid_version: 1.1.20241001
		class ButtonAnim {
		protected:
			ImVec4 ButtonAnimColor = {};
			ImVec2 ButtonAnimSize  = {};

			ImVec4 AnimStatColor[3] = {};
			ImVec2 AnimStatSize[3]  = {};
		public:
			bool DrawButton(const char* name, float speed = 1.0f);

			void SetButtonStatColor(const ImVec4& normal, const ImVec4& hover, const ImVec4& active);
			void SetButtonStatSize(const ImVec2& normal, const ImVec2& hover, const ImVec2& active);
		};
	}
}

#endif