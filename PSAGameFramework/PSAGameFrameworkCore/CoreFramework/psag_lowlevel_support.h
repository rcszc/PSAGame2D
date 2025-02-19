// psag_lowlevel_support. 2024_04_07. RCSZ
// Framework(Engine) LowLevel Module"LLxx" Obj&Func DefineSupport.
// PSA-SUP.1 Update: 2024_04_07

#ifndef __PSAG_LOWLEVEL_SUPPORT_H
#define __PSAG_LOWLEVEL_SUPPORT_H
// LLDefine => LLLogger => Module =>Support.

#ifdef ENABLE_LOWLEVEL_EXT
// framework low-level extension.
#define ENABLE_AVX256_CALC_FUNC
#define ENABLE_PSAG_IMGUI_MATHS
#endif

#include "../LLFilesys/framework_filesys_compress.h"
#include "../LLFilesys/framework_filesys_loader.hpp"
// non-swap: C1189:  gl.h included before glew.h | 20241023.
#include "../LLGraphics/LLGraphicsResource/framework_psaglow_resource.hpp"
#include "../LLSound/framework_sound.h"
#include "../LLThread/framework_thread.hpp"
#include "../LLWindowGui/framework_window.h"

#ifdef PSAG_DEBUG_EXT_MODE
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
#endif
namespace PsagLow {
	// low_module 'graphics' support.
#ifdef ENABLE_LOWMODULE_GRAPHICS
	using PsagSupGraphicsLLRES = PSAGGL_LOWLEVEL::PSAG_GRAPHICS_LLRES;
	// psag renderer tool class&func.
	namespace PsagSupGraphicsOper = PSAGGL_LOWLEVEL::GRAPHICS_OPER;
#endif
	// low_module 'filesys' support.
#ifdef ENABLE_LOWMODULE_FILESYS
	using PsagSupFilesysCompress  = PSAG_FILESYS_COMPR::PsagFilesysCompress;

	namespace PsagSupLoader = PSAG_FILESYS_LOADER;
	namespace PsagSupDCH    = DecoderChannels;
	namespace PsagSupECH    = EncoderChannels;
	namespace PsagSupBase64 = PSAG_FILESYS_BASE64;
#endif

// 宏定义失效问题: "psag_toolkits_engine.h" 定义宏启用底层组件失效.
// 取消组件启用宏. (未解决). [C24-10203], RCSZ 20241024.
	// low_module 'audio' support.
//#ifdef ENABLE_LOWMODULE_AUDIO
	using PsagSupAudioLLRES = PSAGSD_LOWLEVEL::PSAG_AUDIO_LLRES;
	// 'RawDataStream' => 'RawSoundStream'
#define PsagSupAudioRawCVT PSAGSD_LOWLEVEL::AUDIO_LLRES_CONVERT_FUNC

	using PsagSupAudioData       = PSAG_AUDIO_PLAYER::PsagAudioDataResource;
	using PsagSupAudioDataPlayer = PSAG_AUDIO_PLAYER::PsagAudioDataPlayer;

	namespace PsagSupAudioSystem = PSAG_AUDIO_PLAYER::system;
//#endif
	// low_module 'thread' support.
#ifdef ENABLE_LOWMODULE_THREAD
	using PsagSupThreadTasks = PSAG_THREAD_POOL::PsagThreadTasks;
	// return size_t this_thread_id system count.
#define PsagSupThisThreadCountID PSAG_THREAD_POOL::ThisThreadID
#endif
	// low_module 'windowgui' support.
#ifdef ENABLE_LOWMODULE_WINDOWGUI
	using PsagSupWindowGuiConfigWin = PSAG_WINDOW_OGLFW::FwWindowConfig;
	using PsagSupWindowGuiConfigGui = PSAG_WINDOW_IMGUI::ImGuiConfig;

	using PsagSupWindowGuiEventOGLFW = PSAG_WINDOW_OGLFW::PasgWindowEvent;
	using PsagSupWindowGuiEventIMGUI = PSAG_WINDOW_IMGUI::PsagImGuiContextEvent;

	namespace WindowsTools = PSAG_WINDOW_WIN32;
#endif
}

#endif