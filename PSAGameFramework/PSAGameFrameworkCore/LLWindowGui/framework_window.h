// framework_window.

#ifndef _FRAMEWORK_WINDOW_H
#define _FRAMEWORK_WINDOW_H
#include <GLFW/glfw3.h>
#ifdef ENABLE_GLFW_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#ifdef ENABLE_PSAG_IMGUI_MATHS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_glfw3/imgui.h"
#include "imgui_glfw3/imgui_impl_glfw.h"
#include "imgui_glfw3/imgui_impl_opengl3.h"

#if defined(_MSV_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib,"legacy_stdio_definitons.lib")
#endif

// LLWindowGui PSA-L.6 (=>logger).
#include "../LLLogger/framework_logger.hpp"

#define PSAG_WINDOWGUI_LABEL "PSAG_WINDOWGUI"

#define DEVICE_ENABLE_NVIDIA_GPU
namespace PSAG_WINDOW_OGLFW {
	// glfw window init_config. 
	struct FwWindowConfig {
		std::string WindowName;

		uint32_t WindowSizeWidth, WindowSizeHeight;
		bool WindowFullFlag;

		FwWindowConfig() :
			WindowSizeWidth  (768),
			WindowSizeHeight (432),
			WindowName       ({}),
			WindowFullFlag   (false)
		{}
		FwWindowConfig(uint32_t w, uint32_t h, const std::string& name, bool fs) :
			WindowSizeWidth  (w),
			WindowSizeHeight (h),
			WindowName       (name),
			WindowFullFlag   (fs)
		{}
	};

	class SpcaGlfwSystemCallback {
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

		static void CallbackClose(GLFWwindow* window);
		static void CallbackWindowRefresh(GLFWwindow* window);
	};

	using FwSysErrorMessage = const char*;
	// core-framework inherits window-event.
	class SpcaWindowEvent :public SpcaGlfwSystemCallback {
	private:
		static std::chrono::steady_clock::time_point CalcSpeedTimer;
		static size_t CalcSpeedFrameCount;
	protected:
		GLFWwindow* MainWindowObject = {};

		Vector2T<int> RenderBuffer = Vector2T<int>(32, 32);
		Vector4T<float> RenderBgColor = Vector4T<float>();

		FwSysErrorMessage ErrorMessage = nullptr;

		bool GLFWwindowCreate(FwWindowConfig config = {});
		bool GLFWwindowInit(uint32_t version[2], uint32_t MSAA, bool profile, bool fixedsize);

		void GLFWwindowSetIcon(const std::string& file);

		bool GLFWwindowVsync(bool vswitch);
		bool GLFWwindowCallback();

		bool GLFWwindowFree();

		void RenderContextAbove(); // opengl render context above.
		void RenderContextBelow(); // opengl render context below.

		float CalcFrameSpeedScale(float base_fps);
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

	struct ImGuiConfig {
		std::string     ShaderVersionStr;              // opengl glsl version string
		std::string     FontsFilepath;                 // imgui: fonts global read_fIlepath'.ttf'
		Vector4T<float> FontsGlobalColor;              // imgui: fonts global_style_color
		float           FontsGlobalSize;               // imgui: fonts global_style_sizescale
		float           FrameRounding, WindowRounding; // imgui: frame,window global_style_rounding

		ImGuiConfig() :
			ShaderVersionStr ({}),
			FontsFilepath    ({}),
			FontsGlobalColor ({}),
			FontsGlobalSize  (16.0f),
			FrameRounding    (0.0f),
			WindowRounding   (0.0f)
		{}
		ImGuiConfig(
			const std::string& version, const std::string& path, const Vector4T<float>& color, 
			float size, float round, float wround) :
			ShaderVersionStr (version),
			FontsFilepath    (path),
			FontsGlobalColor (color),
			FontsGlobalSize  (size),
			FrameRounding    (round),
			WindowRounding   (wround)
		{}
	};

	class SpcaImGuiEvent {
	protected:
		void ImGuiInit(GLFWwindow* window_object, ImGuiConfig cfgdata = {});
		void ImGuiFree();

		void RenderGuiContextA();
		void RenderGuiContextB();
	};
}

#endif