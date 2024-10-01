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
#include "../LLFilesys/framework_filesys_json.h"
#include "../LLFilesys/framework_filesys_loader.h"
#include "../LLGraphics/LLGraphicsResource/framework_psaglow_resource.hpp"
#include "../LLSound/framework_sound.h"
#include "../LLThread/framework_thread.hpp"
#include "../LLWindowGui/framework_window.h"
#define PSAG_IMGUI_CONTROL_MATHS
#include "../LLWindowGui/framework_window_imgui_ms.h"

namespace PsagLow {
	// low_module 'graphics' support.
#ifdef ENABLE_LOWMODULE_GRAPHICS
	using PsagSupGraphicsLLRES = PSAGGL_LOWLEVEL::PSAG_GRAPHICS_LLRES;
	// psag renderer tool class&func.
	namespace PsagSupGraphicsOper = PSAGGL_LOWLEVEL::GRAPHICS_OPER;
#endif
	// low_module 'filesys' support.
#ifdef ENABLE_LOWMODULE_FILESYS
	using PsagSupFilesysJsonMode = PSAG_FILESYS_JSON::JsonModeType;

	using PsagSupFilesysCompress  = PSAG_FILESYS_COMPR::PsagFilesysCompress;
	using PsagSupFilesysJson      = PSAG_FILESYS_JSON::PsagFilesysJson;
	using PsagSupFilesysLoaderBin = PSAG_FILESYS_LOADER::PsagFilesysLoaderBinary;
	using PsagSupFilesysLoaderStr = PSAG_FILESYS_LOADER::PsagFilesysLoaderString;

	namespace PsagSupFilesysFunc = PSAG_FILESYS_JSON::BASE64_TOOL;
	namespace PsagSupLoader      = PSAG_FILESYS_LOADER;
#endif
	// low_module 'sound' support.
#ifdef ENABLE_LOWMODULE_SOUND
	using PsagSupSoundLLRES = PSAGSD_LOWLEVEL::PSAG_SOUND_LLRES;
	// 'RawDataStream' => 'RawSoundStream'
#define PsagSupSoundRawCVT PSAGSD_LOWLEVEL::SOUND_LLRES_CONVERT_FUNC

	using PsagSupSoundData       = PSAG_SOUND_PLAYER::PsagSoundDataResource;
	using PsagSupSoundDataPlayer = PSAG_SOUND_PLAYER::PsagSoundDataPlayer;

	namespace PsagSupSoundSystem = PSAG_SOUND_PLAYER::system;
#endif
	// low_module 'thread' support.
#ifdef ENABLE_LOWMODULE_THREAD
	using PsagSupThreadTasks = PSAG_THREAD_POOL::PsagThreadTasks;
	// return size_t this_thread_id system count.
#define PsagSupThreadThisIDCount PSAG_THREAD_POOL::ThisThreadID
#endif
	// low_module 'windowgui' support.
#ifdef ENABLE_LOWMODULE_WINDOWGUI
	using PsagSupWindowGuiConfigWin = PSAG_WINDOW_OGLFW::FwWindowConfig;
	using PsagSupWindowGuiConfigGui = PSAG_WINDOW_IMGUI::ImGuiConfig;

	using PsagSupWindowGuiEventOglfw = PSAG_WINDOW_OGLFW::PasgWindowEvent;
	using PsagSupWindowGuiEventImGui = PSAG_WINDOW_IMGUI::PsagImGuiContextEvent;
#endif
}

#endif